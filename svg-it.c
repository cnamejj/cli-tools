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

struct data_pair_list *load_data( struct parsed_options *popt )

{
    int indata, dsize, total = 0, off, nconv, nlines = 0, rc, xcol, ycol,
      minwords, keep;
    float *cx, *cy;
    char databuff[DATABUFFSIZE], *chunk, *alldata, *pos, *source;
    struct data_pair_list *data = 0;
    struct data_block_list *dlist = 0, *walk, *blink;
    struct string_parts *lines = 0, *words = 0;

    /* --- */

    source = popt->data_file;
    xcol = popt->x_col;
    ycol = popt->y_col;

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

    alldata = (char *) malloc( total + 1);
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

    data = (struct data_pair_list *) malloc( sizeof *data );
    if( !data ) bail_out( ERR_MALLOC_FAILED, errno, DO_LOAD_DATA, 0 );

    cx = (float *) malloc( nlines * (sizeof *cx) );
    if( !cx ) bail_out( ERR_MALLOC_FAILED, errno, DO_LOAD_DATA, 0 );

    cy = (float *) malloc( nlines * (sizeof *cy) );
    if( !cy ) bail_out( ERR_MALLOC_FAILED, errno, DO_LOAD_DATA, 0 );

    data->cases = 0;
    data->xval = cx;
    data->yval = cy;

    minwords = xcol;
    if( ycol > minwords ) minwords = ycol;

    for( off = 0; off < nlines; off++ )
    {
        words = explode_string( &rc, lines->list[off], " " );
        if( rc != RC_NORMAL ) bail_out( ERR_INVALID_DATA, 0, DO_LOAD_DATA, "can't break input file into lines for parsing" );

        remove_empty_strings( words );
        if( words->np < minwords)
        {
            if( !popt->ign_bad_data ) bail_out( ERR_INVALID_DATA, 0, DO_LOAD_DATA, "input line does not contain enough fields" );
	}
        else
        {
            keep = 1;

            if( !popt->x_data ) *cx = (float) off;
            else
            {
                nconv = sscanf( words->list[xcol-1], "%f", cx);
/* fprintf(stderr, "dbg:: line:%d xc:%d val'%s' nc:%d f:%f\n", off, xcol, words->list[xcol-1], nconv, *cx); */
                if( !nconv )
                {
                    if( !popt->ign_bad_data ) bail_out( ERR_INVALID_DATA, 0, DO_LOAD_DATA, "input line has non-numeric X value" );
                    keep= 0;
		}
            }

            if( !popt->y_data ) *cy = (float) off;
            else
            {
                nconv = sscanf( words->list[ycol-1], "%f", cy);
/* fprintf(stderr, "dbg:: line:%d yc:%d val'%s' nc:%d f:%f\n", off, ycol, words->list[ycol-1], nconv, *cy); */
                if( !nconv )
                {
                    if( !popt->ign_bad_data ) bail_out( ERR_INVALID_DATA, 0, DO_LOAD_DATA, "input line has non-numeric Y value" );
                    keep= 0;
		}
	    }

            if( keep )
            {
                data->cases++;
                cx++;
                cy++;
	    }
	}
    }

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
    int rc = RC_NORMAL, context, grids, digits, nbyte, svg_doc_len, out;
    float dmin, dmax, span;
    char *dataformat, *svg_doc = 0;
    struct data_pair_list *data = 0;
    struct svg_model *svg;
    struct series_data *series = 0;
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
      { OP_XCOL,        OP_TYPE_INT,  OP_FL_BLANK, FL_XCOL,        0, DEF_XCOL,        0, 0 },
      { OP_YCOL,        OP_TYPE_INT,  OP_FL_BLANK, FL_YCOL,        0, DEF_YCOL,        0, 0 },
      { OP_XDATA,       OP_TYPE_FLAG, OP_FL_BLANK, FL_XDATA,       0, DEF_XDATA,       0, 0 },
      { OP_YDATA,       OP_TYPE_FLAG, OP_FL_BLANK, FL_YDATA,       0, DEF_YDATA,       0, 0 },
      { OP_IG_BAD_DATA, OP_TYPE_FLAG, OP_FL_BLANK, FL_IG_BAD_DATA, 0, DEF_IG_BAD_DATA, 0, 0 },
    };
    struct option_set *co;
    struct parsed_options popt;
    struct word_chain *extra_opts, *walk;
    int nflags = (sizeof opset) / (sizeof opset[0]);

    /* --- */

    popt.debug = popt.help = 0;
    popt.xax_grids = popt.yax_grids = 0;
    popt.x_col = popt.y_col = 0;
    popt.x_data = popt.y_data = 0;
    popt.ign_bad_data = 0;
    popt.chart_title = popt.xax_title = popt.yax_title = 0;
    popt.out_file = popt.data_file = 0;

    context = DO_PARSE_COMMAND;
    extra_opts = parse_command_options( &rc, opset, nflags, narg, opts );

/*dbg*
print_parse_summary( extra_opts, opset, nflags );
 *dbg*/

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
    popt.x_col = *((int *) co->parsed);

    co = get_matching_option( OP_YCOL, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.y_col = *((int *) co->parsed);

    co = get_matching_option( OP_XDATA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.x_data = *((int *) co->parsed);

    co = get_matching_option( OP_YDATA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.y_data = *((int *) co->parsed);

    co = get_matching_option( OP_IG_BAD_DATA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, context, "internal configuration error" );
    popt.ign_bad_data = *((int *) co->parsed);

    if( !popt.chart_title ) popt.chart_title = "";
    if( !popt.xax_title ) popt.xax_title = "";
    if( !popt.yax_title ) popt.yax_title = "";
    if( !popt.out_file ) popt.out_file = "";
    if( !popt.data_file ) popt.data_file = "";

    if( popt.x_col < 1 ) bail_out( ERR_INVALID_DATA, 0, context, "X data column invalid" );
    if( popt.y_col < 1 ) bail_out( ERR_INVALID_DATA, 0, context, "Y data column invalid" );

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

    series = svg_add_float_data( &rc, svg, data->cases, data->xval, data->yval );
    if( rc != RC_NORMAL ) bail_out( rc, 0, context, "unable to add data to chart model" );

    context = DO_CONFIGURE_CHART;

    rc = svg_set_circ_line_size( series, SC_CIRC_LINE_SIZE );
    if( rc == RC_NORMAL ) rc = svg_set_circ_radius( series, SC_CIRC_RADIUS );

    /* --- */

    if( rc == RC_NORMAL ) rc = svg_set_text_color( svg, SC_TEXT_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_axis_color( svg, SC_AXIS_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_chart_color( svg, SC_CHART_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_graph_color( svg, SC_GRAPH_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_circ_fill_color( series, SC_CIRC_FILL_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_circ_line_color( series, SC_CIRC_LINE_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_data_fill_color( series, SC_DATA_FILL_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_data_line_color( series, SC_DATA_LINE_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_x_gridline_color( svg, SC_XGRID_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_y_gridline_color( svg, SC_YGRID_COLOR );

    if( rc == RC_NORMAL ) rc = svg_set_graph_alpha( svg, SC_GRAPH_ALPHA );
    if( rc == RC_NORMAL ) rc = svg_set_circ_fill_alpha( series, SC_CIRC_FILL_ALPHA );
    if( rc == RC_NORMAL ) rc = svg_set_circ_line_alpha( series, SC_CIRC_LINE_ALPHA );
    if( rc == RC_NORMAL ) rc = svg_set_data_fill_alpha( series, SC_DATA_FILL_ALPHA );
    if( rc == RC_NORMAL ) rc = svg_set_data_line_alpha( series, SC_DATA_LINE_ALPHA );

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
