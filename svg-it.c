#define ALLOC_CONTEXT_DATA

#include "cli-sub.h"
#include "svg-tools.h"
#include "svg-it.h"
#include "err_ref.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>

/* --- */

char *context_desc( int context );

void bail_out( int rc, int err, int context, char *explain );

struct data_pair_list *load_data( struct parsed_options *popt );

void expand_series_col_req( struct parsed_options *popt );

int *parse_col_list_req( int *rc, int *ncols, char *req );

/* --- */

char *context_desc( int context )

{
    int which, nref;
    char *desc = 0;
    static char unknown_desc[] = "Unknown context code";

    nref = (sizeof context_list) / (sizeof *context_list);

    for( which = 0; which < nref && !desc; which++ )
      if( context_list[which].ref == context ) desc = context_list[which].desc;

    if( !desc ) desc = unknown_desc;

    return( desc );
}

/* --- */

void bail_out( int rc, int err, int context, char *explain )

{
    fprintf( stderr, "Err(%d) %s", rc, context_desc(context) );

    if( explain ) if( *explain ) fprintf( stderr, ", %s", explain );

    if( err )
    {
        fprintf( stderr, " (%s)", strerror(err) );
    }

    fprintf( stderr, "\n" );

    exit( rc );
}

/* --- */

int *parse_col_list_req( int *rc, int *ncols, char *req )

{
    int *clist = 0, cword, off, nconv, number;
    struct col_pair *cpair = 0, *end = 0, *curr = 0;
    struct string_parts *ents = 0, *range = 0;

    ents = explode_string( rc, req, "," );
    if( *rc != RC_NORMAL ) return( 0 );

    for( cword = 0; cword < ents->np; cword++ )
    {
        curr = (struct col_pair *) malloc( sizeof *cpair );
        if( !curr )
        {
            *rc = ERR_MALLOC_FAILED;
            return( 0 );
	}

        curr->low = 0;
        curr->hi = 0;
        curr->next = 0;
        if( end ) end->next = curr;
        end = curr;
        if( !cpair ) cpair = curr;

        range = explode_string( rc, ents->list[cword], "-" );
        if( *rc != RC_NORMAL ) return( 0 );
        if( range->np > 2 )
        {
            *rc = ERR_UNSUPPORTED;
            return( 0 );
	}

        nconv = sscanf( range->list[0], "%d", &curr->low );
        if( !nconv )
        {
            *rc = ERR_UNSUPPORTED;
            return( 0 );
	}

        if( range->np == 1 ) curr->hi = curr->low;
        else
        {
            nconv = sscanf( range->list[1], "%d", &curr->hi );
            if( !nconv )
            {
                *rc = ERR_UNSUPPORTED;
                return( 0 );
            }
	}

        for( off = 0; off < range->np; off++ ) free( range->list[off] );
        free( range );
    }

    for( off = 0; off < ents->np; off++ ) free( ents->list[off] );
    free( ents );

    *ncols = 0;
    for( curr = cpair; curr; curr = curr->next )
    {
        if( curr->hi < curr->low )
        {
            *rc = ERR_UNSUPPORTED;
            return( 0 );
    	}

        *ncols += (curr->hi - curr->low) + 1;
    }


    clist = (int *) malloc( *ncols * (sizeof *clist) );
    if( !clist )
    {
        *rc = ERR_MALLOC_FAILED;
        return( 0 );
    }

    off = 0;
    for( curr = cpair; curr; curr = curr->next )
    {
        for( number = curr->low; number <= curr->hi; number++ )
        {
            clist[off] = number;
            off++;
	}
    }

    return( clist );
}

/* --- */

void expand_series_col_req( struct parsed_options *popt )

{
    int nx = 0, ny = 0, *xlist = 0, *ylist = 0, rc = RC_NORMAL, curr, bigger, *cols, **extend, off;

    if( !popt->x_data && !popt->y_data ) return;

    if( popt->x_data )
    {
        if( !*popt->x_col_req ) bail_out( ERR_UNSUPPORTED, 0, DO_PARSE_COMMAND, "No X data columns given" );
        xlist = parse_col_list_req( &rc, &nx, popt->x_col_req );
        if( rc != RC_NORMAL ) bail_out( ERR_MALLOC_FAILED, errno, DO_PARSE_COMMAND, "" );
    }

    if( popt->y_data )
    {
        if( !*popt->y_col_req ) bail_out( ERR_UNSUPPORTED, 0, DO_PARSE_COMMAND, "No Y data columns given" );
        ylist = parse_col_list_req( &rc, &ny, popt->y_col_req );
        if( rc != RC_NORMAL ) bail_out( ERR_MALLOC_FAILED, errno, DO_PARSE_COMMAND, "" );
    }

    if( nx && ny && nx != ny )
    {
        if( ny > nx )
        {
            curr = ny;
            bigger = nx;
            extend = &xlist;
	}
        else
        {
            curr = nx;
            bigger = ny;
            extend = &ylist;
	}

        cols = (int *) malloc( bigger * (sizeof *cols) );
        if( !cols ) bail_out( ERR_MALLOC_FAILED, errno, DO_PARSE_COMMAND, "" );

        for( off = 0; off < curr; off++ ) cols[off] = *extend[off];
        for( off = curr; off <= bigger; off++ ) cols[off] = *extend[curr];
        free( *extend );
        *extend = cols;
    }

    if( ny > nx ) popt->nseries = ny;
    else popt->nseries = nx;

    popt->x_col_list = xlist;
    popt->y_col_list = ylist;

    return;
}

/* --- */

struct data_pair_list *load_data( struct parsed_options *popt )

{
    int indata, dsize, total = 0, off, nconv, nlines = 0, rc, *xcols, *ycols,
      minwords, keep, nseries, snum, cl, cases;
    float *cx, *cy;
    char databuff[DATABUFFSIZE], *chunk, *alldata, *pos, *source, *delim;
    struct data_pair_list *data = 0;
    struct data_block_list *dlist = 0, *walk, *blink;
    struct string_parts *lines = 0, *words = 0;

    /* --- */

    source = popt->data_file;
    xcols = popt->x_col_list;
    ycols = popt->y_col_list;
    nseries = popt->nseries;
    delim = popt->delim;

    if( !strcmp( source, IS_STDIN ) ) indata = fileno( stdin );
    else
    {
        indata = open( source, DATA_OPEN_FLAGS );
        if( indata == -1 ) bail_out( ERR_OPEN_FAILED, errno, DO_LOAD_DATA, "can't open data file" );
    }

    dsize = read( indata, databuff, DATABUFFSIZE );
    for( ; dsize > 0; dsize = read( indata, databuff, DATABUFFSIZE ) )
    {
        blink = (struct data_block_list *) malloc( sizeof *blink );
        chunk = (char *) malloc( dsize );
        if( !chunk || !blink ) bail_out( ERR_MALLOC_FAILED, errno, DO_LOAD_DATA, 0 );

        total += dsize;
        memcpy( chunk, databuff, dsize );
        blink->data = chunk;
        blink->size = dsize;
        blink->next = 0;

        if( !dlist ) dlist = walk = blink;
        else
        {
            walk->next = blink;
            walk = blink;
        }
    }

    alldata = (char *) malloc( total + 1 );
    if( !alldata ) bail_out( ERR_MALLOC_FAILED, errno, DO_LOAD_DATA, 0 );

    pos = alldata;
    for( walk = dlist; walk; walk = walk->next )
    {
        memcpy( pos, walk->data, walk->size );
        pos += walk->size;
    }

    *pos = '\0';

    for( walk = dlist; walk; )
    {
        blink = walk->next;
        free( walk->data);
        free( walk);
        walk = blink;
    }

    lines = explode_string( &rc, alldata, "\n" );
    if( rc != RC_NORMAL) bail_out( ERR_INVALID_DATA, 0, DO_LOAD_DATA, "can't break input file into lines for parsing" );

    remove_empty_strings( lines );
    nlines = lines->np;
    if( nlines < 1 ) bail_out( ERR_INVALID_DATA, 0, DO_LOAD_DATA, "no LF's in input file" );

    free( alldata );

    data = (struct data_pair_list *) malloc( nseries * (sizeof *data) );
    if( !data ) bail_out( ERR_MALLOC_FAILED, errno, DO_LOAD_DATA, 0 );

    for( off = 0; off < nseries; off++ )
    {
        cx = (float *) malloc( nlines * (sizeof *cx) );
        if( !cx ) bail_out( ERR_MALLOC_FAILED, errno, DO_LOAD_DATA, 0 );

        cy = (float *) malloc( nlines * (sizeof *cy) );
        if( !cy ) bail_out( ERR_MALLOC_FAILED, errno, DO_LOAD_DATA, 0 );

        data[off].cases = 0;
        data[off].xval = cx;
        data[off].yval = cy;
    }

    minwords = 1;
    for( off = 0; off < nseries; off++ )
    {
        if( popt->x_data ) if( xcols[off] > minwords ) minwords = xcols[off];
        if( popt->y_data ) if( ycols[off] > minwords ) minwords = ycols[off];
    }

    cases = 0;
    for( cl = 0; cl < nlines; cl++ )
    {
        words = explode_string( &rc, lines->list[cl], delim );
        if( rc != RC_NORMAL ) bail_out( ERR_INVALID_DATA, 0, DO_LOAD_DATA, "can't break input file into lines for parsing" );

        if( !strcmp(delim, IS_BLANK) ) remove_empty_strings( words );

        if( words->np < minwords)
        {
            if( !popt->ign_bad_data ) bail_out( ERR_INVALID_DATA, 0, DO_LOAD_DATA, "input line does not contain enough fields" );
	}
        else
        {
            keep = 1;

            for( snum = 0; snum < nseries; snum++ )
            {
                cx = &data[snum].xval[cases];
                if( !popt->x_data ) *cx = (float) cases;
                else 
                {
                    nconv = sscanf( words->list[xcols[snum]-1], "%f", cx);
                    if( popt->debug ) fprintf( stderr, "dbg:: Load-Data: X-data rec #%d case #%d series #%d raw(%s) nc: %d co: %f\n",
                      cl, cases, snum, words->list[xcols[snum]-1], nconv, data[snum].xval[cases] );
                    if( !nconv )
                    {
                        if( !popt->ign_bad_data ) bail_out( ERR_INVALID_DATA, 0, DO_LOAD_DATA, "input line has non-numeric X value" );
                        keep= 0;
		    }
		}

                cy = &data[snum].yval[cases];
                if( !popt->y_data ) *cy = (float) cases;
                else
                {
                    nconv = sscanf( words->list[ycols[snum]-1], "%f", cy);
                    if( popt->debug ) fprintf( stderr, "dbg:: Load-Data: Y-data rec #%d case #%d series #%d raw(%s) nc: %d co: %f\n",
                      cl, cases, snum, words->list[ycols[snum]-1], nconv, data[snum].yval[cases] );
                    if( !nconv )
                    {
                        if( !popt->ign_bad_data ) bail_out( ERR_INVALID_DATA, 0, DO_LOAD_DATA, "input line has non-numeric Y value" );
                        keep= 0;
		    }
		}
	    }

            if( keep ) cases++;

	}
    }

    for( snum = 0; snum < nseries; snum++ ) data[snum].cases = cases;

    if( lines )
    {
        for( off = 0; off < lines->np; off++) free( lines->list[off] );
        free( lines->list );
        free( lines );
    }

    /* --- */

    return( data );
}

/* --- */

int main( int narg, char **opts )

{
    int rc = RC_NORMAL, context, grids, digits, nbyte, svg_doc_len, out, snum;
    float dmin, dmax, span;
    char *dataformat, *svg_doc = 0;
    struct data_pair_list *data = 0;
    struct svg_model *svg = 0;
    struct series_data *ds = 0;
    static struct option_set opset[] = {
      { OP_DEBUG,       OP_TYPE_INT,  OP_FL_BLANK, FL_DEBUG,       0, DEF_DEBUG,       0, 0 },
      { OP_HELP,        OP_TYPE_FLAG, OP_FL_BLANK, FL_HELP,        0, DEF_HELP,        0, 0 },
      { OP_CHART_TITLE, OP_TYPE_CHAR, OP_FL_BLANK, FL_CHART_TITLE, 0, DEF_CHART_TITLE, 0, 0 },
      { OP_XAX_TITLE,   OP_TYPE_CHAR, OP_FL_BLANK, FL_XAX_TITLE,   0, DEF_XAX_TITLE,   0, 0 },
      { OP_YAX_TITLE,   OP_TYPE_CHAR, OP_FL_BLANK, FL_YAX_TITLE,   0, DEF_YAX_TITLE,   0, 0 },
      { OP_XAX_GRIDS,   OP_TYPE_INT,  OP_FL_BLANK, FL_XAX_GRIDS,   0, DEF_XAX_GRIDS,   0, 0 },
      { OP_YAX_GRIDS,   OP_TYPE_INT,  OP_FL_BLANK, FL_YAX_GRIDS,   0, DEF_YAX_GRIDS,   0, 0 },
      { OP_OUTFILE,     OP_TYPE_CHAR, OP_FL_BLANK, FL_OUTFILE,     0, DEF_OUTFILE,     0, 0 },
      { OP_DATAFILE,    OP_TYPE_CHAR, OP_FL_BLANK, FL_DATAFILE,    0, DEF_DATAFILE,    0, 0 },
      { OP_XCOL,        OP_TYPE_CHAR, OP_FL_BLANK, FL_XCOL,        0, DEF_XCOL,        0, 0 },
      { OP_YCOL,        OP_TYPE_CHAR, OP_FL_BLANK, FL_YCOL,        0, DEF_YCOL,        0, 0 },
      { OP_XDATA,       OP_TYPE_FLAG, OP_FL_BLANK, FL_XDATA,       0, DEF_XDATA,       0, 0 },
      { OP_YDATA,       OP_TYPE_FLAG, OP_FL_BLANK, FL_YDATA,       0, DEF_YDATA,       0, 0 },
      { OP_IG_BAD_DATA, OP_TYPE_FLAG, OP_FL_BLANK, FL_IG_BAD_DATA, 0, DEF_IG_BAD_DATA, 0, 0 },
      { OP_DATA_DELIM,  OP_TYPE_CHAR, OP_FL_BLANK, FL_DATA_DELIM,  0, DEF_DATA_DELIM,  0, 0 },
    };
    struct option_set *co;
    struct parsed_options popt;
    struct word_chain *extra_opts, *walk;
    int nflags = (sizeof opset) / (sizeof opset[0]);

#ifdef DEBUG_MALLOC
/* bug_control( BUG_FLAG_SET, BUG_OPT_OBSESSIVE | BUG_OPT_TRCALLS | BUG_OPT_TRFREE | BUG_OPT_KEEPONFREE | BUG_OPT_REINITONFREE ); */
 bug_control( BUG_FLAG_SET, BUG_OPT_TRCALLS | BUG_OPT_OBSESSIVE | BUG_OPT_TRFREE );
/*  bug_control( BUG_FLAG_SET, BUG_OPT_KEEPONFREE | BUG_OPT_STRICT_FREE | BUG_OPT_TRCALLS | BUG_OPT_OBSESSIVE | BUG_OPT_TRFREE ); */
#endif


    /* --- */

    popt.debug = popt.help = 0;
    popt.xax_grids = popt.yax_grids = 0;
    popt.nseries = 0;
    popt.x_col_list = popt.y_col_list = 0;
    popt.x_col_req = popt.y_col_req = 0;
    popt.x_data = popt.y_data = 0;
    popt.ign_bad_data = 0;
    popt.chart_title = popt.xax_title = popt.yax_title = 0;
    popt.out_file = popt.data_file = 0;
    popt.delim = 0;

    context = DO_PARSE_COMMAND;
    extra_opts = parse_command_options( &rc, opset, nflags, narg, opts );

/*dbg*/
print_parse_summary( extra_opts, opset, nflags );
/*dbg*/

    for( walk = extra_opts; walk; walk = walk->next)
      if( walk->opt ) if( *walk->opt ) bail_out( ERR_SYNTAX, 0, context, "extraneous parameters on commandline" );
    
    /* --- */

    co = get_matching_option( OP_DEBUG, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.debug = *((int *) co->parsed);

    co = get_matching_option( OP_HELP, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.help = *((int *) co->parsed);

    co = get_matching_option( OP_CHART_TITLE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.chart_title = (char *) co->parsed;

    co = get_matching_option( OP_XAX_TITLE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.xax_title = (char *) co->parsed;

    co = get_matching_option( OP_YAX_TITLE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.yax_title = (char *) co->parsed;
    
    co = get_matching_option( OP_XAX_GRIDS, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.xax_grids = *((int *) co->parsed);

    co = get_matching_option( OP_YAX_GRIDS, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.yax_grids = *((int *) co->parsed);

    co = get_matching_option( OP_OUTFILE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.out_file = (char *) co->parsed;

    co = get_matching_option( OP_DATAFILE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.data_file = (char *) co->parsed;

    co = get_matching_option( OP_XCOL, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.x_col_req = (char *) co->parsed;

    co = get_matching_option( OP_YCOL, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.y_col_req = (char *) co->parsed;

    co = get_matching_option( OP_XDATA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.x_data = *((int *) co->parsed);

    co = get_matching_option( OP_YDATA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.y_data = *((int *) co->parsed);

    co = get_matching_option( OP_IG_BAD_DATA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.ign_bad_data = *((int *) co->parsed);

    co = get_matching_option( OP_DATA_DELIM, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.delim = (char *) co->parsed;

    if( !popt.chart_title ) popt.chart_title = "";
    if( !popt.xax_title ) popt.xax_title = "";
    if( !popt.yax_title ) popt.yax_title = "";
    if( !popt.out_file ) popt.out_file = "";
    if( !popt.data_file ) popt.data_file = "";
    if( !popt.x_col_req ) popt.x_col_req = "";
    if( !popt.y_col_req ) popt.x_col_req = "";
    if( !popt.delim ) popt.delim = "";

    expand_series_col_req( &popt );

    if( narg < 2 ) popt.help = 1;

    /* --- */

    if( popt.help )
    {
        printf( SHOW_SYNTAX, opts[ 0 ] );
        exit( 0 );
    }
    
    /* --- */

    context = DO_ALLOC_CHART_OBJECT;
    svg = svg_make_chart();
    if( !svg ) bail_out( ERR_MALLOC_FAILED, errno, context, 0 );

    context = DO_LOAD_DATA;
    data = load_data( &popt );

    if( popt.debug )
    {
        int off;

        for( off = 0; off < data[0].cases; off++ )
        {
            fprintf( stderr, "dbg:: Rec#%d ", off );
            for( snum = 0; snum < popt.nseries; snum++ )
              fprintf( stderr, " %f/%f", data[snum].xval[off], data[snum].yval[off] );
            fprintf( stderr, "\n");
	}
    }

    for( snum = 0; snum < popt.nseries; snum++ )
    {
        if( data[snum].cases < 1 ) bail_out( ERR_UNSUPPORTED, 0, context, "Empty data series cannot be charted" );
        ds = svg_add_float_data( &rc, svg, data[snum].cases, data[snum].xval, data[snum].yval );
        if( rc != RC_NORMAL ) bail_out( rc, 0, context, "unable to add data to chart model" );
    }

    context = DO_CONFIGURE_CHART;

    /* --- */

    if( rc == RC_NORMAL ) rc = svg_set_text_color( svg, SC_TEXT_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_axis_color( svg, SC_AXIS_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_chart_color( svg, SC_CHART_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_graph_color( svg, SC_GRAPH_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_x_gridline_color( svg, SC_XGRID_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_y_gridline_color( svg, SC_YGRID_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_graph_alpha( svg, SC_GRAPH_ALPHA );
    for( ds = svg->series; ds; ds = ds->next )
    {
        if( rc == RC_NORMAL ) rc = svg_set_circ_radius( ds, SC_CIRC_RADIUS );
        if( rc == RC_NORMAL ) rc = svg_set_circ_line_size( ds, SC_CIRC_LINE_SIZE );
        if( rc == RC_NORMAL ) rc = svg_set_circ_fill_color( ds, SC_CIRC_FILL_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_circ_line_color( ds, SC_CIRC_LINE_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_data_fill_color( ds, SC_DATA_FILL_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_data_line_color( ds, SC_DATA_LINE_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_circ_fill_alpha( ds, SC_CIRC_FILL_ALPHA );
        if( rc == RC_NORMAL ) rc = svg_set_circ_line_alpha( ds, SC_CIRC_LINE_ALPHA );
        if( rc == RC_NORMAL ) rc = svg_set_data_fill_alpha( ds, SC_DATA_FILL_ALPHA );
        if( rc == RC_NORMAL ) rc = svg_set_data_line_alpha( ds, SC_DATA_LINE_ALPHA );
    }

    if( rc != RC_NORMAL ) bail_out( rc, 0, context, "setting chart color/alpha options failed" );

    /* --- */

    rc = svg_set_screen_width( svg, "100%" );
    if( rc == RC_NORMAL ) rc = svg_set_screen_height( svg, "50%" );

    if( rc == RC_NORMAL ) rc = svg_set_chart_title( svg, popt.chart_title );
    if( rc == RC_NORMAL ) rc = svg_set_xax_title( svg, popt.xax_title );
    if( rc == RC_NORMAL ) rc = svg_set_yax_title( svg, popt.yax_title );

    if( rc == RC_NORMAL && popt.xax_grids > 0 ) rc = svg_set_xax_num_grids( svg, popt.xax_grids );
    if( rc == RC_NORMAL && popt.yax_grids > 0 ) rc = svg_set_yax_num_grids( svg, popt.yax_grids );

    if( rc != RC_NORMAL ) bail_out( rc, 0, context, "setting chart size and detail options failed" );

    /* --- */

    rc = svg_finalize_model( svg );
    if( rc != RC_NORMAL ) bail_out( rc, 0, context, "error finalizeing SVG model" );

    dmin = svg_get_xmin( svg );
    dmax = svg_get_xmax( svg );
    grids = svg_get_xax_num_grids( svg );
    if( grids > 0 )
    {
        span = (dmax - dmin) / grids;
        if( span >= 1.0 ) digits = 0;
        else if( span == 0.0 ) digits = 0;
        else digits = (int) (1 - log10( span));
        dataformat = string_from_int( &rc, digits, LABEL_META_FORMAT );
        rc = svg_set_xax_disp( svg, dataformat );
        if( rc != RC_NORMAL ) bail_out( rc, 0, context, "can't set x-axis label precision" );
        if( dataformat ) free( dataformat );
    }

    context = DO_RENDER_CHART;
    svg_doc = svg_render( &rc, svg );
    if( rc != RC_NORMAL) bail_out( rc, errno, context, "chart rendering error" );

    context = DO_OPEN_OUTPUT_FILE;

    if( *popt.out_file && strcmp(popt.out_file, IS_STDOUT) )
    {
        out = open( popt.out_file, OUT_OPEN_FLAGS, OUT_OPEN_MODE );
        if( out == -1 ) bail_out( ERR_OPEN_FAILED, errno, context, "can't open output file" );
    }
    else out = fileno( stdout );

    context = DO_WRITE_SVG_DOC;
    svg_doc_len = strlen(svg_doc);
    nbyte = write( out, svg_doc, svg_doc_len );
    if( nbyte != svg_doc_len ) bail_out( ERR_WRITE_FAILED, errno, context, "writing SVG document failed" );

    svg_free_model( svg );

    /* --- */

    return( rc );
}
