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

#include <ctype.h>

/* --- */

void set_label_format_type( struct parsed_options *popt )

{
    int format_type[2], off;
    char *tick_format[2], *sp;

    tick_format[0] = popt->xtick_format;
    tick_format[1] = popt->ytick_format;

    format_type[0] = DTYPE_AUTO;
    format_type[1] = DTYPE_AUTO;

    for( off = 0; off <= 1; off++)
    {
        sp = tick_format[off];

        if( !strncmp( sp, IS_DTYPE_TIME, strlen(IS_DTYPE_TIME) ) )
        {
            tick_format[off] += strlen(IS_DTYPE_TIME);
            format_type[off] = DTYPE_TIME;
	}
        else if( !strncmp( sp, IS_DTYPE_FLOAT, strlen(IS_DTYPE_FLOAT) ) )
        {
            tick_format[off] += strlen(IS_DTYPE_FLOAT);
            format_type[off] = DTYPE_FLOAT;
	}
        else if( !strncmp( sp, IS_DTYPE_FIXED, strlen(IS_DTYPE_FIXED) ) )
        {
            tick_format[off] += strlen(IS_DTYPE_FIXED);
            format_type[off] = DTYPE_FIXED;
	}
    }

    popt->xtick_format = tick_format[0];
    popt->xtick_type = format_type[0];

    popt->ytick_format = tick_format[1];
    popt->ytick_type = format_type[1];

    return;
}

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

char *comm_op( int html_out )

{
    static char *op = "<!-- ", eos = '\0';

    if( html_out ) return( op );
    else return( &eos );
}

/* --- */

char *comm_cl( int html_out )

{
    static char *cl = " -->", eos = '\0';

    if( html_out ) return( cl );
    else return( &eos );
}

/* --- */

void bail_out( int rc, int err, int html_out, int context, char *explain )

{
    FILE *errout;

    if( html_out ) errout = stdout;
    else errout = stderr;

    /* --- */

    if( html_out ) printf( "%s%s", SVG_TEXT_HEADER, SVG_TEXT_PREFIX );

    fprintf( errout, "Err(%d) %s", rc, context_desc(context) );

    if( explain ) if( *explain ) fprintf( errout, ", %s", explain );

    if( err )
    {
        fprintf( errout, " (%s)", strerror(err) );
    }

    fprintf( errout, "\n" );

    if( html_out ) printf( "%s%s", SVG_TEXT_SUFFIX, SVG_TEXT_TRAILER );

    exit( rc );
}

/* --- */

void show_form_and_exit()

{
    printf( "%s", construct_entry_form( HTML_FORM_TEMPLATE ) );

    exit( 0 );
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
        if( nconv != 1 )
        {
            *rc = ERR_UNSUPPORTED;
            return( 0 );
	}

        if( range->np == 1 ) curr->hi = curr->low;
        else
        {
            nconv = sscanf( range->list[1], "%d", &curr->hi );
            if( nconv != 1 )
            {
                *rc = ERR_UNSUPPORTED;
                return( 0 );
            }
	}

        for( off = 0; off < range->np; off++ )
        {
            free( range->list[off] );
            range->list[off] = 0;
	}
        free( range->list );
        range->list = 0;
        free( range );
    }

    for( off = 0; off < ents->np; off++ )
    {
        free( ents->list[off] );
        ents->list[off] = 0;
    }
    free( ents->list );
    ents->list = 0;
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

    /* --- */

    for( end = cpair; end; )
    {
        curr = end;
        end = curr->next;

        curr->next = 0;
        free( curr );
    }

    curr = end = cpair = 0;

    /* --- */

    return( clist );
}

/* --- */

void expand_series_col_req( struct parsed_options *popt )

{
    int nx = 0, ny = 0, *xlist = 0, *ylist = 0, rc = RC_NORMAL, curr, bigger, *cols, **extend, off;

    if( !popt->x_data && !popt->y_data ) return;

    if( popt->x_data )
    {
        if( !*popt->x_col_req ) bail_out( ERR_UNSUPPORTED, 0, popt->html_out, DO_PARSE_COMMAND, "No X data columns given" );
        xlist = parse_col_list_req( &rc, &nx, popt->x_col_req );
        if( rc != RC_NORMAL ) bail_out( ERR_MALLOC_FAILED, errno, popt->html_out, DO_PARSE_COMMAND, "" );
    }

    if( popt->y_data )
    {
        if( !*popt->y_col_req ) bail_out( ERR_UNSUPPORTED, 0, popt->html_out, DO_PARSE_COMMAND, "No Y data columns given" );
        ylist = parse_col_list_req( &rc, &ny, popt->y_col_req );
        if( rc != RC_NORMAL ) bail_out( ERR_MALLOC_FAILED, errno, popt->html_out, DO_PARSE_COMMAND, "" );
    }

    if( nx && ny && nx != ny )
    {
        if( ny > nx )
        {
            curr = nx;
            bigger = ny;
            extend = &xlist;
	}
        else
        {
            curr = ny;
            bigger = nx;
            extend = &ylist;
	}

        cols = (int *) malloc( bigger * (sizeof *cols) );
        if( !cols ) bail_out( ERR_MALLOC_FAILED, errno, popt->html_out, DO_PARSE_COMMAND, "" );

        for( off = 0; off < curr; off++ ) cols[off] = *extend[off];
        for( off = curr; off < bigger; off++ ) cols[off] = *extend[curr-1];
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

char *gen_data_series_desc( int *rc, int dsid, int xin, int yin )

{
    char *desc = 0, *seg = 0, *agg = 0, *combo = 0;

    if( *rc == RC_NORMAL )
    {
        seg = string_from_int( rc, dsid, 0 );
        agg = combine_strings( rc, DS_DESC_ID_PREF, seg );
        if( seg )
        {
            free( seg );
            seg = 0;
	}
        combo = combine_strings( rc, agg, DS_DESC_ID_SUFF );
        if( agg ) free( agg );
        agg = combo;
        combo = 0; 

        if( xin != NO_VALUE )
        {
            combo = combine_strings( rc, agg, DS_DESC_XCOL_PREF );
            if( agg ) free( agg );
            agg = combo;

            seg = string_from_int( rc, xin, 0 );
            combo = combine_strings( rc, agg, seg );
            if( agg ) free( agg );
            if( seg ) free( seg );
            agg = combo;

            seg = combo = 0;
	}

        if( yin != NO_VALUE )
        {
            combo = combine_strings( rc, agg, DS_DESC_YCOL_PREF );
            if( agg ) free( agg );
            agg = combo;

            seg = string_from_int( rc, yin, 0 );
            combo = combine_strings( rc, agg, seg );
            if( agg ) free( agg );
            if( seg ) free( seg );
            agg = combo;

            seg = combo = 0;
	}

        if( *rc == RC_NORMAL ) desc = agg;
        else if( agg ) free( agg );
        agg = 0;
    }

    /* --- */

    if( !desc ) desc = strdup( NO_DESCRIPTION );

    if( !desc ) *rc = ERR_MALLOC_FAILED;

    return( desc );
}

/* --- */

struct data_pair_list *load_data( struct parsed_options *popt )

{
    int indata, dsize, total = 0, off, nconv, nlines = 0, rc, *xcols, *ycols,
      minwords, keep, nseries, snum, cl, all_good, xword, yword, xin_col,
      yin_col;
    double *cx, *cy;
    char databuff[DATABUFFSIZE], *chunk, *alldata, *pos, *source, *delim;
    struct data_pair_list *data = 0;
    struct data_block_list *dlist = 0, *walk, *blink;
    struct string_parts *lines = 0, *words = 0;
    FILE *errout;

    /* --- */

    source = popt->data_file;
    xcols = popt->x_col_list;
    ycols = popt->y_col_list;
    nseries = popt->nseries;
    delim = popt->delim;

    if( popt->html_out ) errout = stdout;
    else errout = stderr;

    if( !*popt->raw_data )
    {
        if( !strcmp( source, IS_STDIN ) ) indata = fileno( stdin );
        else
        {
            indata = open( source, DATA_OPEN_FLAGS );
            if( indata == -1 ) bail_out( ERR_OPEN_FAILED, errno, popt->html_out, DO_LOAD_DATA, "can't open data file" );
        }

        dsize = read( indata, databuff, DATABUFFSIZE );
        for( ; dsize > 0; dsize = read( indata, databuff, DATABUFFSIZE ) )
        {
            blink = (struct data_block_list *) malloc( sizeof *blink );
            chunk = (char *) malloc( dsize );
            if( !chunk || !blink ) bail_out( ERR_MALLOC_FAILED, errno, popt->html_out, DO_LOAD_DATA, 0 );

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
        if( !alldata ) bail_out( ERR_MALLOC_FAILED, errno, popt->html_out, DO_LOAD_DATA, 0 );

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
        if( rc != RC_NORMAL) bail_out( ERR_INVALID_DATA, 0, popt->html_out, DO_LOAD_DATA, "can't break input file into lines for parsing" );

        free( alldata );
    }

    else
    {
        lines = explode_string( &rc, popt->raw_data, popt->raw_eol );
        if( rc != RC_NORMAL) bail_out( ERR_INVALID_DATA, 0, popt->html_out, DO_LOAD_DATA, "can't break input file into lines for parsing" );
    }

    remove_empty_strings( lines );
    nlines = lines->np;
    if( nlines < 1 ) bail_out( ERR_INVALID_DATA, 0, popt->html_out, DO_LOAD_DATA, "no LF's in input file" );

    data = (struct data_pair_list *) malloc( nseries * (sizeof *data) );
    if( !data ) bail_out( ERR_MALLOC_FAILED, errno, popt->html_out, DO_LOAD_DATA, 0 );

    for( off = 0; off < nseries && rc == RC_NORMAL; off++ )
    {
        cx = (double *) malloc( nlines * (sizeof *cx) );
        if( !cx ) bail_out( ERR_MALLOC_FAILED, errno, popt->html_out, DO_LOAD_DATA, 0 );

        cy = (double *) malloc( nlines * (sizeof *cy) );
        if( !cy ) bail_out( ERR_MALLOC_FAILED, errno, popt->html_out, DO_LOAD_DATA, 0 );

        data[off].cases = 0;
        data[off].xval = cx;
        data[off].yval = cy;

        if( !popt->x_data ) xin_col = NO_VALUE;
        else xin_col = xcols[off];

        if( !popt->y_data ) yin_col = NO_VALUE;
        else yin_col = ycols[off];

        data[off].desc = gen_data_series_desc( &rc, off + 1, xin_col, yin_col );
        data[off].named = 0;
    }

    minwords = 1;
    for( off = 0; off < nseries; off++ )
    {
        if( popt->x_data ) if( xcols[off] > minwords ) minwords = xcols[off];
        if( popt->y_data ) if( ycols[off] > minwords ) minwords = ycols[off];
    }

    for( cl = 0; cl < nlines; cl++ )
    {
        words = explode_string( &rc, lines->list[cl], delim );
        if( rc != RC_NORMAL ) bail_out( ERR_INVALID_DATA, 0, popt->html_out, DO_LOAD_DATA, "can't break input line into words for parsing" );

        if( !strcmp(delim, IS_BLANK) ) remove_empty_strings( words );

        if( words->np < minwords)
        {
            if( popt->debug ) printf( "%sdbg:: Load-Data: Not enough words in rec #%d, wanted %d and found %d%s\n", comm_op(popt->html_out), cl,
              minwords, words->np, comm_cl(popt->html_out) );
            if( !popt->ign_bad_data ) bail_out( ERR_INVALID_DATA, 0, popt->html_out, DO_LOAD_DATA, "input line does not contain enough fields" );
	}

        all_good = 1;

        for( snum = 0; snum < nseries; snum++ )
        {
            keep = 1;
            if( popt->x_data ) xword = xcols[snum] - 1;
            if( popt->y_data ) yword = ycols[snum] - 1;

            cx = &data[snum].xval[data[snum].cases];
            if( !popt->x_data ) *cx = (double) data[snum].cases;
            else if( xword >= words->np ) keep = 0;
            else 
            {
                nconv = sscanf( words->list[xword], "%lf", cx);
                if( popt->debug ) fprintf( errout, "%sdbg:: Load-Data: X-data rec #%d case #%d series #%d raw(%s) nc: %d co: %f%s\n",
                  comm_op(popt->html_out), cl, data[snum].cases, snum, words->list[xword], nconv, data[snum].xval[data[snum].cases], comm_cl(popt->html_out) );
                if( nconv != 1 )
                {
                    if( !popt->ign_bad_data ) bail_out( ERR_INVALID_DATA, 0, popt->html_out, DO_LOAD_DATA, "input line has non-numeric X value" );
                    keep = 0;
                }
            }

            cy = &data[snum].yval[data[snum].cases];
            if( !popt->y_data ) *cy = (double) data[snum].cases;
            else if( yword >= words->np ) keep = 0;
            else
            {
                nconv = sscanf( words->list[yword], "%lf", cy);
                if( popt->debug ) fprintf( errout, "%sdbg:: Load-Data: Y-data rec #%d case #%d series #%d raw(%s) nc: %d co: %f%s\n",
                  comm_op(popt->html_out), cl, data[snum].cases, snum, words->list[yword], nconv, data[snum].yval[data[snum].cases], comm_cl(popt->html_out) );
                if( nconv != 1 )
                {
                    if( !popt->ign_bad_data ) bail_out( ERR_INVALID_DATA, 0, popt->html_out, DO_LOAD_DATA, "input line has non-numeric Y value" );
                    keep = 0;
                }
            }

            if( !popt->only_all_good && keep ) data[snum].cases++;
            if( all_good ) all_good = keep;
        }

        if( popt->only_all_good && all_good ) for( snum = 0; snum < nseries; snum++ ) data[snum].cases++;

        if( words)
        {
            for( off = 0; off < words->np; off++) free( words->list[off] );
            free( words->list );
            free( words );
            words = 0;
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
    int rc = RC_NORMAL, context, grids, digits, nbyte, svg_doc_len, out, snum,
      nseries_styles, fl_circ_alpha, is_cgi, show_form, use, dtype;
    double dmin, dmax, span;
    double no_value = (double) SVG_NO_VALUE;
    char *dataformat, *svg_doc = 0, *st, *cgi_data, *cgi_raw_eol = 0, *cli_raw_eol = 0,
      *def_data_delim = 0, empty_string[] = { '\0' }, *split, *desc;
    struct data_pair_list *data = 0;
    struct svg_model *svg = 0;
    struct series_data *ds = 0;
    struct data_series_visuals *viz = 0;
    struct value_chain *nlist, *top;
    static struct option_set opset[] = {
      { OP_DEBUG,       OP_TYPE_INT,   OP_FL_BLANK,   FL_DEBUG,       0, DEF_DEBUG,       0, 0 },
      { OP_HELP,        OP_TYPE_FLAG,  OP_FL_BLANK,   FL_HELP,        0, DEF_HELP,        0, 0 },
      { OP_CHART_TITLE, OP_TYPE_CHAR,  OP_FL_BLANK,   FL_CHART_TITLE, 0, DEF_CHART_TITLE, 0, 0 },
      { OP_XAX_TITLE,   OP_TYPE_CHAR,  OP_FL_BLANK,   FL_XAX_TITLE,   0, DEF_XAX_TITLE,   0, 0 },
      { OP_YAX_TITLE,   OP_TYPE_CHAR,  OP_FL_BLANK,   FL_YAX_TITLE,   0, DEF_YAX_TITLE,   0, 0 },
      { OP_XAX_GRIDS,   OP_TYPE_INT,   OP_FL_BLANK,   FL_XAX_GRIDS,   0, DEF_XAX_GRIDS,   0, 0 },
      { OP_YAX_GRIDS,   OP_TYPE_INT,   OP_FL_BLANK,   FL_YAX_GRIDS,   0, DEF_YAX_GRIDS,   0, 0 },
      { OP_OUTFILE,     OP_TYPE_CHAR,  OP_FL_BLANK,   FL_OUTFILE,     0, DEF_OUTFILE,     0, 0 },
      { OP_DATAFILE,    OP_TYPE_CHAR,  OP_FL_BLANK,   FL_DATAFILE,    0, DEF_DATAFILE,    0, 0 },
      { OP_XCOL,        OP_TYPE_CHAR,  OP_FL_BLANK,   FL_XCOL,        0, DEF_XCOL,        0, 0 },
      { OP_YCOL,        OP_TYPE_CHAR,  OP_FL_BLANK,   FL_YCOL,        0, DEF_YCOL,        0, 0 },
      { OP_XDATA,       OP_TYPE_FLAG,  OP_FL_BLANK,   FL_XDATA,       0, DEF_XDATA,       0, 0 },
      { OP_YDATA,       OP_TYPE_FLAG,  OP_FL_BLANK,   FL_YDATA,       0, DEF_YDATA,       0, 0 },
      { OP_IG_BAD_DATA, OP_TYPE_FLAG,  OP_FL_BLANK,   FL_IG_BAD_DATA, 0, DEF_IG_BAD_DATA, 0, 0 },
      { OP_DATA_DELIM,  OP_TYPE_CHAR,  OP_FL_BLANK,   FL_DATA_DELIM,  0, DEF_DATA_DELIM,  0, 0 },
      { OP_CIRC_ALPHA,  OP_TYPE_FLOAT, OP_FL_BLANK,   FL_CIRC_ALPHA,  0, DEF_CIRC_ALPHA,  0, 0 },
      { OP_DATA_ALPHA,  OP_TYPE_FLOAT, OP_FL_BLANK,   FL_DATA_ALPHA,  0, DEF_DATA_ALPHA,  0, 0 },
      { OP_CFILL_ALPHA, OP_TYPE_FLOAT, OP_FL_BLANK,   FL_CFILL_ALPHA, 0, DEF_CFILL_ALPHA, 0, 0 },
      { OP_DFILL_ALPHA, OP_TYPE_FLOAT, OP_FL_BLANK,   FL_DFILL_ALPHA, 0, DEF_DFILL_ALPHA, 0, 0 },
      { OP_CIRC_RADIUS, OP_TYPE_INT,   OP_FL_BLANK,   FL_CIRC_RADIUS, 0, DEF_CIRC_RADIUS, 0, 0 },
      { OP_CIRC_LSIZE,  OP_TYPE_INT,   OP_FL_BLANK,   FL_CIRC_LSIZE,  0, DEF_CIRC_LSIZE,  0, 0 },
      { OP_DATA_LSIZE,  OP_TYPE_INT,   OP_FL_BLANK,   FL_DATA_LSIZE,  0, DEF_DATA_LSIZE,  0, 0 },
      { OP_KP_ALL_GOOD, OP_TYPE_FLAG,  OP_FL_BLANK,   FL_KP_ALL_GOOD, 0, DEF_KP_ALL_GOOD, 0, 0 },
      { OP_XMIN_VAL,    OP_TYPE_FLOAT, OP_FL_BLANK,   FL_XMIN_VAL,    0, DEF_XMIN_VAL,    0, 0 },
      { OP_XMAX_VAL,    OP_TYPE_FLOAT, OP_FL_BLANK,   FL_XMAX_VAL,    0, DEF_XMAX_VAL,    0, 0 },
      { OP_YMIN_VAL,    OP_TYPE_FLOAT, OP_FL_BLANK,   FL_YMIN_VAL,    0, DEF_YMIN_VAL,    0, 0 },
      { OP_YMAX_VAL,    OP_TYPE_FLOAT, OP_FL_BLANK,   FL_YMAX_VAL,    0, DEF_YMAX_VAL,    0, 0 },
      { OP_WIDTH,       OP_TYPE_INT,   OP_FL_BLANK,   FL_WIDTH,       0, DEF_WIDTH,       0, 0 },
      { OP_HEIGHT,      OP_TYPE_INT,   OP_FL_BLANK,   FL_HEIGHT,      0, DEF_HEIGHT,      0, 0 },
      { OP_DISP_WIDTH,  OP_TYPE_CHAR,  OP_FL_BLANK,   FL_DISP_WIDTH,  0, DEF_DISP_WIDTH,  0, 0 },
      { OP_DISP_HEIGHT, OP_TYPE_CHAR,  OP_FL_BLANK,   FL_DISP_HEIGHT, 0, DEF_DISP_HEIGHT, 0, 0 },
      { OP_RAW_DATA,    OP_TYPE_CHAR,  OP_FL_BLANK,   FL_RAW_DATA,    0, DEF_RAW_DATA,    0, 0 },
      { OP_RAW_EOL,     OP_TYPE_CHAR,  OP_FL_BLANK,   FL_RAW_EOL,     0, DEF_RAW_EOL,     0, 0 },
      { OP_LEGEND,      OP_TYPE_FLAG,  OP_FL_BLANK,   FL_LEGEND,      0, DEF_LEGEND,      0, 0 },
      { OP_LSCALE,      OP_TYPE_INT,   OP_FL_BLANK,   FL_LSCALE,      0, DEF_LSCALE,      0, 0 },
      { OP_DSNAME,      OP_TYPE_CHAR,  OP_FL_REPEATS, FL_DSNAME,      0, DEF_DSNAME,      0, 0 },
      { OP_XLAB_FORMAT, OP_TYPE_CHAR,  OP_FL_BLANK,   FL_XLAB_FORMAT, 0, DEF_XLAB_FORMAT, 0, 0 },
      { OP_YLAB_FORMAT, OP_TYPE_CHAR,  OP_FL_BLANK,   FL_YLAB_FORMAT, 0, DEF_YLAB_FORMAT, 0, 0 },
    };
    struct option_set *co, *co_leg;
    struct parsed_options popt;
    struct word_chain *extra_opts, *walk;
    int nflags = (sizeof opset) / (sizeof opset[0]);
    FILE *errout;

#ifdef DEBUG_MALLOC
  bug_control( BUG_FLAG_SET, BUG_OPT_TRCALLS | BUG_OPT_OBSESSIVE | BUG_OPT_TRFREE | BUG_OPT_KEEPONFREE );
/*  bug_control( BUG_FLAG_SET, BUG_OPT_TRCALLS | BUG_OPT_OBSESSIVE | BUG_OPT_TRFREE | BUG_OPT_KEEPONFREE | BUG_OPT_REINITONFREE ); */
/*    bug_control( BUG_FLAG_SET, BUG_OPT_TRCALLS | BUG_OPT_OBSESSIVE | BUG_OPT_TRFREE ); */
/*  bug_control( BUG_FLAG_SET, BUG_OPT_TRCALLS | BUG_OPT_OBESSSIVE | BUG_OPT_TRFREE | BUG_OPT_KEEPONFREE | BUG_OPT_STRICT_FREE ); */
#endif

    nseries_styles = (sizeof def_series_visuals) / (sizeof def_series_visuals[0]);

    /* --- */

    cgi_raw_eol = strdup( CGI_RAW_EOL );
    cli_raw_eol = strdup( CLI_RAW_EOL );
    def_data_delim = strdup( DEF_DATA_DELIM );

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
    popt.circ_line_alpha = popt.circ_fill_alpha = popt.data_line_alpha = popt.data_fill_alpha = 0.0;
    popt.circ_radius = popt.circ_line_size = popt.data_line_size = 0;
    popt.only_all_good = 0;
    popt.fix_xmin = popt.fix_xmax = popt.fix_ymin = popt.fix_ymax = no_value;
    popt.chart_width = popt.chart_height = SVG_NO_VALUE;
    popt.display_width = popt.display_height = 0;
    popt.html_out = 0;
    popt.raw_data = 0;
    popt.raw_eol = 0;
    popt.has_legend = 0;
    popt.legend_scale = 0;
    popt.dsname = 0;
    popt.xtick_type = popt.ytick_type = 0;
    popt.xtick_format = popt.ytick_format = 0;

    context = DO_PARSE_COMMAND;

    is_cgi = called_as_cgi();
    if( is_cgi )
    {
        popt.html_out = 1;
        errout = stdout;
        cgi_data = get_cgi_data( &rc );
        if( !cgi_data ) show_form = 1;
        else if( !*cgi_data ) show_form = 1;
        else show_form = 0;

        if( show_form ) show_form_and_exit();

        printf( "%s", SVG_RESPONSE_HEADER );
        extra_opts = parse_cgi_options( &rc, opset, nflags, cgi_data );
    }
    else
    {
        errout = stderr;
        extra_opts = parse_command_options( &rc, opset, nflags, narg, opts );
    }

    co = get_matching_option( OP_DEBUG, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.debug = *((int *) co->parsed);

    if( popt.debug && !popt.html_out ) print_parse_summary( extra_opts, opset, nflags );

    for( walk = extra_opts; walk; walk = walk->next)
      if( walk->opt ) if( *walk->opt ) bail_out( ERR_SYNTAX, 0, popt.html_out, context, "extraneous parameters on commandline" );
    
    /* --- */

    co = get_matching_option( OP_HELP, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.help = *((int *) co->parsed);

    co = get_matching_option( OP_CHART_TITLE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.chart_title = (char *) co->parsed;

    co = get_matching_option( OP_XAX_TITLE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.xax_title = (char *) co->parsed;

    co = get_matching_option( OP_YAX_TITLE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.yax_title = (char *) co->parsed;
    
    co = get_matching_option( OP_XAX_GRIDS, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.xax_grids = *((int *) co->parsed);

    co = get_matching_option( OP_YAX_GRIDS, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.yax_grids = *((int *) co->parsed);

    co = get_matching_option( OP_OUTFILE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.out_file = (char *) co->parsed;

    co = get_matching_option( OP_DATAFILE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.data_file = (char *) co->parsed;

    co = get_matching_option( OP_XCOL, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.x_col_req = (char *) co->parsed;

    co = get_matching_option( OP_YCOL, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.y_col_req = (char *) co->parsed;

    co = get_matching_option( OP_XDATA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.x_data = *((int *) co->parsed);

    co = get_matching_option( OP_YDATA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.y_data = *((int *) co->parsed);

    co = get_matching_option( OP_IG_BAD_DATA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.ign_bad_data = *((int *) co->parsed);

    co = get_matching_option( OP_DATA_DELIM, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.delim = (char *) co->parsed;

    co = get_matching_option( OP_CIRC_ALPHA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.circ_line_alpha = *((float *) co->parsed);
    fl_circ_alpha = co->flags & OP_FL_FOUND;

    co = get_matching_option( OP_DATA_ALPHA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.data_line_alpha = *((float *) co->parsed);

    co = get_matching_option( OP_CFILL_ALPHA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.circ_fill_alpha = *((float *) co->parsed);
    if( !co->flags & OP_FL_FOUND && fl_circ_alpha ) popt.circ_fill_alpha = popt.circ_line_alpha;

    co = get_matching_option( OP_DFILL_ALPHA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.data_fill_alpha = *((float *) co->parsed);

    co = get_matching_option( OP_CIRC_RADIUS, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.circ_radius = *((int *) co->parsed);

    co = get_matching_option( OP_CIRC_LSIZE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.circ_line_size = *((int *) co->parsed);

    co = get_matching_option( OP_DATA_LSIZE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.data_line_size = *((int *) co->parsed);

    co = get_matching_option( OP_KP_ALL_GOOD, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.only_all_good = *((int *) co->parsed);

    co = get_matching_option( OP_XMIN_VAL, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.fix_xmin = *((float *) co->parsed);

    co = get_matching_option( OP_XMAX_VAL, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.fix_xmax = *((float *) co->parsed);

    co = get_matching_option( OP_YMIN_VAL, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.fix_ymin = *((float *) co->parsed);

    co = get_matching_option( OP_YMAX_VAL, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.fix_ymax = *((float *) co->parsed);

    co = get_matching_option( OP_WIDTH, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.chart_width = *((int *) co->parsed);

    co = get_matching_option( OP_HEIGHT, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.chart_height = *((int *) co->parsed);

    co = get_matching_option( OP_DISP_WIDTH, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.display_width = (char *) co->parsed;

    co = get_matching_option( OP_DISP_HEIGHT, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.display_height = (char *) co->parsed;

    co = get_matching_option( OP_RAW_DATA, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.raw_data = (char *) co->parsed;

    co = get_matching_option( OP_RAW_EOL, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.raw_eol = (char *) co->parsed;

    co = get_matching_option( OP_LEGEND, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.has_legend = *((int *) co->parsed);
    co_leg = co;

    co = get_matching_option( OP_LSCALE, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.legend_scale = *((int *) co->parsed);
    if( co->flags & OP_FL_FOUND )
    {
        if( !(co_leg->flags & OP_FL_FOUND) || co->opt_num > co_leg->opt_num )
        {
            /* This test is to make CGI sub'd requests work in a sane way */
            if( !is_cgi || *co->val ) popt.has_legend = !!popt.legend_scale;
	}
    }

    co = get_matching_option( OP_XLAB_FORMAT, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.xtick_format = (char *) co->parsed;

    co = get_matching_option( OP_YLAB_FORMAT, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.ytick_format = (char *) co->parsed;

    co = get_matching_option( OP_DSNAME, opset, nflags );
    if( !co ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "internal configuration error" );
    popt.dsname = (struct value_chain *) co->parsed;

    if( !popt.chart_title ) popt.chart_title = empty_string;
    if( !popt.xax_title ) popt.xax_title = empty_string;
    if( !popt.yax_title ) popt.yax_title = empty_string;
    if( !popt.out_file ) popt.out_file = empty_string;
    if( !popt.data_file ) popt.data_file = empty_string;
    if( !popt.x_col_req ) popt.x_col_req = empty_string;
    if( !popt.y_col_req ) popt.x_col_req = empty_string;
    if( !popt.display_width ) popt.display_width = empty_string;
    if( !popt.display_height ) popt.display_height = empty_string;
    if( !popt.raw_data ) popt.raw_data = empty_string;
    if( !popt.raw_eol ) popt.raw_eol = empty_string;
    if( !popt.xtick_format ) popt.xtick_format = empty_string;
    if( !popt.ytick_format ) popt.xtick_format = empty_string;

    if( !popt.delim ) popt.delim = empty_string;
    if( !*popt.delim ) popt.delim = def_data_delim;

    /* --- */

    set_label_format_type( &popt );

    /* --- */

    if( is_cgi )
    {
        popt.data_file = empty_string;
        popt.out_file = empty_string;
        popt.help = 0;
        if( !*popt.raw_eol ) popt.raw_eol = cgi_raw_eol;
        if( popt.xax_grids < 1 ) popt.xax_grids = SVG_NO_VALUE;
        if( popt.yax_grids < 1 ) popt.yax_grids = SVG_NO_VALUE;
        if( !popt.chart_width ) popt.chart_width = SVG_NO_VALUE;
        if( !popt.chart_height ) popt.chart_height = SVG_NO_VALUE;
    }
    else
    {
        if( !*popt.raw_eol ) popt.raw_eol = cli_raw_eol;
        if( narg < 2 ) popt.help = 1;
    }

    if( popt.help )
    {
        printf( SHOW_SYNTAX, opts[ 0 ] );
        exit( 0 );
    }

    /* ---
     * If we're going to accept partial records, that implies that we will accept
     * records with invalid data.
     */
    if( !popt.only_all_good ) popt.ign_bad_data = 1;

    expand_series_col_req( &popt );

    /* --- */

    context = DO_ALLOC_CHART_OBJECT;
    svg = svg_make_chart();
    if( !svg ) bail_out( ERR_MALLOC_FAILED, errno, popt.html_out, context, 0 );

    context = DO_LOAD_DATA;
    data = load_data( &popt );

    if( popt.debug )
    {
        int off;

        for( off = 0; off < data[0].cases; off++ )
        {
            fprintf( errout, "%sdbg:: Rec#%d ", comm_op(popt.html_out), off );
            for( snum = 0; snum < popt.nseries; snum++ )
              fprintf( errout, " %f/%f", data[snum].xval[off], data[snum].yval[off] );
            fprintf( errout, "%s\n", comm_cl(popt.html_out) );
	}
    }

    /* --- */

    top = popt.dsname;
    
    for( nlist = popt.dsname; nlist && rc == RC_NORMAL; nlist = nlist->next )
    {
        /* Make sure this flag is set so we can use it to mark entries we've processed */
        nlist->flags |= OP_FL_SET;

        desc = (char *) nlist->parsed;
        if( *desc == DS_ID_MARK )
        {
            snum = strtoul( desc + 1, &split, BASE10 );
            if( 0 < snum && snum <= popt.nseries )
            {
                nlist->flags &= ~OP_FL_SET;
                snum--;
                for( ; *split == ' '; split++ ) ;
                if( data[snum].desc ) free( data[snum].desc );
                data[snum].named = 1;
                data[snum].desc = strdup( split );
                if( !data[snum].desc ) rc = ERR_MALLOC_FAILED;
	    }
	}
        else if( !*desc ) nlist->flags &= ~OP_FL_SET;

        if( nlist->flags & OP_FL_SET )
          if( !(top->flags & OP_FL_SET) || nlist->opt_num < top->opt_num ) top = nlist;
    }

    /* This is ugly, since the list of options is reversed from what we want */

    if( top) for( ; top->flags & OP_FL_SET; )
    {
        top->flags &= ~OP_FL_SET;

        for( use = 1, snum = 0; use && snum < popt.nseries && rc == RC_NORMAL; snum++ )
        {
            if( !data[snum].named )
            {
                if( data[snum].desc ) free( data[snum].desc );
                data[snum].named = 1;
                data[snum].desc = strdup( (char *) top->parsed );
                if( !data[snum].desc ) rc = ERR_MALLOC_FAILED;
                use = 0;
	    }
	}

        for( nlist = popt.dsname; nlist; nlist = nlist->next )
          if( nlist->flags & OP_FL_SET )
             if( !(top->flags & OP_FL_SET) || nlist->opt_num < top->opt_num ) top = nlist;
    }

    /* --- */

    for( snum = 0; snum < popt.nseries; snum++ )
    {
        if( data[snum].cases < 1 ) bail_out( ERR_UNSUPPORTED, 0, popt.html_out, context, "empty data series cannot be charted" );
        ds = svg_add_double_data( &rc, svg, data[snum].cases, data[snum].xval, data[snum].yval );
        if( rc != RC_NORMAL ) bail_out( rc, 0, popt.html_out, context, "unable to add data to chart model" );
        rc = svg_set_data_desc( ds, data[snum].desc );
    }

    context = DO_CONFIGURE_CHART;

    /* --- */

    if( rc == RC_NORMAL ) rc = svg_set_xax_format_type( svg, popt.xtick_type );
    if( rc == RC_NORMAL ) rc = svg_set_xax_disp( svg, popt.xtick_format );
    if( rc == RC_NORMAL ) rc = svg_set_yax_format_type( svg, popt.ytick_type );
    if( rc == RC_NORMAL ) rc = svg_set_yax_disp( svg, popt.ytick_format );

    if( rc == RC_NORMAL ) rc = svg_set_text_color( svg, SC_TEXT_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_axis_color( svg, SC_AXIS_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_chart_color( svg, SC_CHART_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_graph_color( svg, SC_GRAPH_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_x_gridline_color( svg, SC_XGRID_COLOR );
    if( rc == RC_NORMAL ) rc = svg_set_y_gridline_color( svg, SC_YGRID_COLOR );

    if( rc == RC_NORMAL ) rc = svg_set_graph_alpha( svg, SC_GRAPH_ALPHA );
    for( ds = svg->series; ds; ds = ds->next )
    {
        viz = &def_series_visuals[(ds->id - 1) % nseries_styles];
        if( popt.debug ) fprintf( errout, "%sdbg:: Visual-Config: series #%d setting %d of %d, cf/cl/dl %s/%s/%s%s\n",
          comm_op(popt.html_out), ds->id, (ds->id-1) % nseries_styles, nseries_styles, viz->circle_fill, viz->circle_line, viz->data_line, comm_cl(popt.html_out) );
        if( rc == RC_NORMAL ) rc = svg_set_circ_radius( ds, popt.circ_radius );

        if( rc == RC_NORMAL ) rc = svg_set_data_line_size( ds, popt.data_line_size );
        if( rc == RC_NORMAL ) rc = svg_set_circ_line_size( ds, popt.circ_line_size );
        if( rc == RC_NORMAL ) rc = svg_set_circ_fill_color( ds, viz->circle_fill );
        if( rc == RC_NORMAL ) rc = svg_set_circ_line_color( ds, viz->circle_line );
        if( rc == RC_NORMAL ) rc = svg_set_data_fill_color( ds, SC_DATA_FILL_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_data_line_color( ds, viz->data_line );
        if( rc == RC_NORMAL ) rc = svg_set_circ_fill_alpha( ds, popt.circ_fill_alpha );
        if( rc == RC_NORMAL ) rc = svg_set_circ_line_alpha( ds, popt.circ_line_alpha );
        if( rc == RC_NORMAL ) rc = svg_set_data_fill_alpha( ds, popt.data_fill_alpha );
        if( rc == RC_NORMAL ) rc = svg_set_data_line_alpha( ds, popt.data_line_alpha );
    }

    if( rc != RC_NORMAL ) bail_out( rc, 0, popt.html_out, context, "setting chart color/alpha options failed" );

    /* --- */

    st = popt.display_width;
    if( !*st ) st = DEF_DISP_WIDTH;
    rc = svg_set_screen_width( svg, st );

    st = popt.display_height;
    if( !*st ) st = DEF_DISP_HEIGHT;
    if( rc == RC_NORMAL ) rc = svg_set_screen_height( svg, st );

    if( rc == RC_NORMAL )
    {
        svg_set_has_legend( svg, popt.has_legend );
        svg_set_legend_scale( svg, popt.legend_scale );
    }

    if( rc == RC_NORMAL ) rc = svg_set_chart_title( svg, popt.chart_title );
    if( rc == RC_NORMAL ) rc = svg_set_xax_title( svg, popt.xax_title );
    if( rc == RC_NORMAL ) rc = svg_set_yax_title( svg, popt.yax_title );

    if( rc == RC_NORMAL && popt.xax_grids > 0 ) rc = svg_set_xax_num_grids( svg, popt.xax_grids );
    if( rc == RC_NORMAL && popt.yax_grids > 0 ) rc = svg_set_yax_num_grids( svg, popt.yax_grids );

    if( rc == RC_NORMAL && popt.fix_xmin != no_value ) rc = svg_set_xmin( svg, popt.fix_xmin );
    if( rc == RC_NORMAL && popt.fix_xmax != no_value ) rc = svg_set_xmax( svg, popt.fix_xmax );
    if( rc == RC_NORMAL && popt.fix_ymin != no_value ) rc = svg_set_ymin( svg, popt.fix_ymin );
    if( rc == RC_NORMAL && popt.fix_ymax != no_value ) rc = svg_set_ymax( svg, popt.fix_ymax );

    if( rc == RC_NORMAL && popt.chart_width != SVG_NO_VALUE ) rc = svg_set_chart_width( svg, popt.chart_width );
    if( rc == RC_NORMAL && popt.chart_height != SVG_NO_VALUE ) rc = svg_set_chart_height( svg, popt.chart_height );

    if( popt.debug )
    {
        struct series_data *dds;

        fprintf( errout, "%sdbg:: Overall, X: min/max: %f/%f, Y: min/max: %f/%f%s\n", comm_op(popt.html_out), svg->xmin, svg->xmax, svg->ymin, svg->ymax, comm_cl(popt.html_out) );
        for( dds = svg->series; dds; dds = dds->next )
          fprintf( errout, "%sdbg:: id: %d, X: min/max: %f/%f, Y: min/max: %f/%f%s\n", comm_op(popt.html_out), dds->id, dds->loc_xmin, dds->loc_xmax, dds->loc_ymin, dds->loc_ymax,
            comm_cl(popt.html_out) );
    }

    if( rc != RC_NORMAL ) bail_out( rc, 0, popt.html_out, context, "setting chart size and detail options failed" );

    /* --- */

    rc = svg_finalize_model( svg );
    if( rc != RC_NORMAL ) bail_out( rc, 0, popt.html_out, context, "error finalizeing SVG model" );

    if( popt.debug )
    {
        struct series_data *dds;

        fprintf( errout, "%sdbg:: Overall, X: min/max: %f/%f, Y: min/max: %f/%f%s\n", comm_op(popt.html_out), svg->xmin, svg->xmax, svg->ymin, svg->ymax, comm_cl(popt.html_out) );
        for( dds = svg->series; dds; dds = dds->next )
          fprintf( errout, "%sdbg:: id: %d, X: min/max: %f/%f, Y: min/max: %f/%f%s\n", comm_op(popt.html_out), dds->id, dds->loc_xmin, dds->loc_xmax,
            dds->loc_ymin, dds->loc_ymax, comm_cl(popt.html_out) );
    }

    dmin = svg_get_xmin( svg );
    dmax = svg_get_xmax( svg );
    grids = svg_get_xax_num_grids( svg );
    dtype = svg_get_xax_format_type( svg );
    if( grids > 0 && dtype == DTYPE_AUTO )
    {
        span = (dmax - dmin) / grids;
        if( span >= 1.0 ) digits = 0;
        else if( span == 0.0 ) digits = 0;
        else digits = (int) (1 - log10( span));
        dataformat = string_from_int( &rc, digits, LABEL_META_FORMAT );
        rc = svg_set_xax_disp( svg, dataformat );
        if( rc != RC_NORMAL ) bail_out( rc, 0, popt.html_out, context, "can't set x-axis label precision" );
        if( dataformat ) free( dataformat );
    }

    context = DO_RENDER_CHART;
    svg_doc = svg_render( &rc, svg );
    if( rc != RC_NORMAL) bail_out( rc, errno, popt.html_out, context, "chart rendering error" );

    context = DO_OPEN_OUTPUT_FILE;

    if( *popt.out_file && strcmp(popt.out_file, IS_STDOUT) )
    {
        out = open( popt.out_file, OUT_OPEN_FLAGS, OUT_OPEN_MODE );
        if( out == -1 ) bail_out( ERR_OPEN_FAILED, errno, popt.html_out, context, "can't open output file" );
    }
    else out = fileno( stdout );

    context = DO_WRITE_SVG_DOC;

    if( popt.html_out ) fflush( stdout );

    svg_doc_len = strlen(svg_doc);
    nbyte = write( out, svg_doc, svg_doc_len );
    if( nbyte != svg_doc_len ) bail_out( ERR_WRITE_FAILED, errno, popt.html_out, context, "writing SVG document failed" );

    svg_free_model( svg );

    /* --- */

    if( svg_doc )
    {
        free( svg_doc );
        svg_doc = 0;
    }

    free_loaded_data( data, popt.nseries );
    data = 0;

    free_command_flags( opset, nflags );

    clear_parsed_options( &popt );

    free( cgi_raw_eol );
    cgi_raw_eol = 0;

    free( cli_raw_eol );
    cli_raw_eol = 0;

    popt.raw_eol = 0;

    free( def_data_delim );
    def_data_delim = 0;

    /* --- */

    return( rc );
}

/* --- */

void free_loaded_data( struct data_pair_list *data, int nseries )

{
    int seq = 0;

    if( data )
    {
        for( seq = 0; seq < nseries; seq++ )
        {
            if( data[seq].xval )
            {
                free( data[seq].xval );
                data[seq].xval = 0;
	    }

            if( data[seq].yval )
            {
                free( data[seq].yval );
                data[seq].yval = 0;
	    }

            if( data[seq].desc )
            {
                free( data[seq].desc );
                data[seq].desc = 0;
	    }
	}

        free( data );
    }

    return;
}

/* --- */

void free_command_flags( struct option_set *ops, int nflags )

{
    int seq = nflags;

    for( seq--; seq >= 0; seq-- )
    {
        if( ops[seq].val ) free( ops[seq].val );

        if( ops[seq].flags & OP_FL_REPEATS ) free_value_chain( ops[seq].parsed );
        else if( ops[seq].parsed ) free( ops[seq].parsed );

        ops[seq].val = 0;
        ops[seq].parsed = 0;
    }

    return;
}

/* --- */

void clear_parsed_options( struct parsed_options *popt )

{
    if( popt )
    {
        if( popt->x_col_list ) free( popt->x_col_list );
        if( popt->y_col_list ) free( popt->y_col_list );

        popt->x_col_list = 0;
        popt->y_col_list = 0;
        popt->data_file = 0;
        popt->out_file = 0;
        popt->chart_title = 0;
        popt->xax_title = 0;
        popt->yax_title = 0;
        popt->x_col_req = 0;
        popt->y_col_req = 0;
        popt->delim = 0;
        popt->display_width = 0;
        popt->display_height = 0;
        popt->raw_data = 0;
        popt->raw_eol = 0;
        popt->xtick_format = 0;
        popt->ytick_format = 0;
    }

    return;
}
