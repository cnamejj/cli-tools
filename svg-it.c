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

struct data_pair_list *load_data( int *rc, char *source )

{
    int indata, dsize, total = 0, off, nconv, nlines = 0;
    float *cx, *cy;
    char databuff[DATABUFFSIZE], *chunk, *alldata, *pos;
    struct data_pair_list *data = 0;
    struct data_block_list *dlist = 0, *walk, *blink;
    struct string_parts *lines = 0;

    /* --- */

    if( *rc == RC_NORMAL )
    {
        if( !strcmp( source, IS_STDIN ) ) indata = fileno( stdin );
        else
        {
            indata = open( source, DATA_OPEN_FLAGS );
            if( indata == -1 ) *rc = ERR_OPEN_FAILED;
	}
    }

    if( *rc == RC_NORMAL )
    {
        dsize = read( indata, databuff, DATABUFFSIZE );
        for( ; *rc == RC_NORMAL && dsize > 0; dsize = read( indata, databuff, DATABUFFSIZE ) )
        {
            blink = (struct data_block_list *) malloc( sizeof *blink );
            chunk = (char *) malloc( dsize );
            if( !chunk || !blink ) *rc = ERR_MALLOC_FAILED;
            else
            {
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
	}
    }

    if( *rc == RC_NORMAL )
    {
        alldata = (char *) malloc( total + 1);
        if( !alldata ) *rc = ERR_MALLOC_FAILED;
        else
        {
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
	}
    }

    if( *rc == RC_NORMAL )
    {
        lines = explode_string( rc, alldata, "\n" );
        if( *rc == RC_NORMAL)
        {
            nlines = lines->np - 1;
            if( nlines < 1 ) *rc = ERR_INVALID_DATA;
	}

        free( alldata );
    }

    if( *rc == RC_NORMAL)
    {
        data = (struct data_pair_list *) malloc( sizeof *data );
        if( !data ) *rc = ERR_MALLOC_FAILED;
    }

    if( *rc == RC_NORMAL)
    {
        cx = (float *) malloc( nlines * (sizeof *cx) );
        if( !cx ) *rc = ERR_MALLOC_FAILED;
    }

    if( *rc == RC_NORMAL)
    {
        cy = (float *) malloc( nlines * (sizeof *cy) );
        if( !cy ) *rc = ERR_MALLOC_FAILED;
    }

    if( *rc == RC_NORMAL )
    {
        data->cases = 0;
        data->xval = cx;
        data->yval = cy;

        for( off = 0; *rc == RC_NORMAL && off < nlines; off++ )
        {
            nconv = sscanf( lines->list[off], "%f %f", cx, cy);
            if( nconv != 2 ) *rc = ERR_INVALID_DATA;
            else
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
    };
    struct option_set *co;
    struct parsed_options popt;
    struct word_chain *extra_opts, *walk;
    int nflags = (sizeof opset) / (sizeof opset[0]);

    /* --- */

    popt.debug = popt.help = popt.xax_grids = popt.yax_grids = 0;
    popt.chart_title = popt.xax_title = popt.yax_title = popt.out_file = popt.data_file = 0;

    context = DO_PARSE_COMMAND;
    extra_opts = parse_command_options( &rc, opset, nflags, narg, opts );

/*dbg*
print_parse_summary( extra_opts, opset, nflags );
 *dbg*/

    for( walk = extra_opts; rc == RC_NORMAL && walk; walk = walk->next)
      if( walk->opt ) if( *walk->opt ) rc = ERR_SYNTAX;
    
    /* --- */

    if( rc == RC_NORMAL )
    {
        co = get_matching_option( OP_DEBUG, opset, nflags );
        if( !co ) rc = ERR_UNSUPPORTED;
        else popt.debug = *((int *) co->parsed);
    }

    if( rc == RC_NORMAL )
    {
        co = get_matching_option( OP_HELP, opset, nflags );
        if( !co ) rc = ERR_UNSUPPORTED;
        else popt.help = *((int *) co->parsed);
    }

    if( rc == RC_NORMAL )
    {
        co = get_matching_option( OP_CHART_TITLE, opset, nflags );
        if( !co ) rc = ERR_UNSUPPORTED;
        else popt.chart_title = (char *) co->parsed;
    }

    if( rc == RC_NORMAL )
    {
        co = get_matching_option( OP_XAX_TITLE, opset, nflags );
        if( !co ) rc = ERR_UNSUPPORTED;
        else popt.xax_title = (char *) co->parsed;
    }

    if( rc == RC_NORMAL )
    {
        co = get_matching_option( OP_YAX_TITLE, opset, nflags );
        if( !co ) rc = ERR_UNSUPPORTED;
        else popt.yax_title = (char *) co->parsed;
    }
    
    if( rc == RC_NORMAL )
    {
        co = get_matching_option( OP_XAX_GRIDS, opset, nflags );
        if( !co ) rc = ERR_UNSUPPORTED;
        else popt.xax_grids = *((int *) co->parsed);
    }
    
    if( rc == RC_NORMAL )
    {
        co = get_matching_option( OP_YAX_GRIDS, opset, nflags );
        if( !co ) rc = ERR_UNSUPPORTED;
        else popt.yax_grids = *((int *) co->parsed);
    }

    if( rc == RC_NORMAL )
    {
        co = get_matching_option( OP_OUTFILE, opset, nflags );
        if( !co ) rc = ERR_UNSUPPORTED;
        else popt.out_file = (char *) co->parsed;
    }

    if( rc == RC_NORMAL )
    {
        co = get_matching_option( OP_DATAFILE, opset, nflags );
        if( !co ) rc = ERR_UNSUPPORTED;
        else popt.data_file = (char *) co->parsed;
    }

    if( rc == RC_NORMAL )
    {
        if( !popt.chart_title ) popt.chart_title = "";
        if( !popt.xax_title ) popt.xax_title = "";
        if( !popt.yax_title ) popt.yax_title = "";
        if( !popt.out_file ) popt.out_file = "";
        if( !popt.data_file ) popt.data_file = "";
    }
    
    /* --- */

    if( rc == RC_NORMAL )
    {
        context = DO_ALLOC_CHART_OBJECT;
        svg = svg_make_chart();
        if( !svg ) rc = ERR_MALLOC_FAILED;
    }

    if( rc == RC_NORMAL )
    {
        context = DO_LOAD_DATA;
        data = load_data( &rc, popt.data_file );
    }

    if( rc == RC_NORMAL )
    {
        rc = svg_add_float_data( svg, data->cases, data->xval, data->yval );
    }

    if( rc == RC_NORMAL )
    {
        context = DO_CONFIGURE_CHART;

        rc = svg_set_circ_line_size( svg, SC_CIRC_LINE_SIZE );
        if( rc == RC_NORMAL ) rc = svg_set_circ_radius( svg, SC_CIRC_RADIUS );

        if( rc == RC_NORMAL ) rc = svg_set_text_color( svg, SC_TEXT_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_axis_color( svg, SC_AXIS_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_chart_color( svg, SC_CHART_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_graph_color( svg, SC_GRAPH_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_circ_fill_color( svg, SC_CIRC_FILL_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_circ_line_color( svg, SC_CIRC_LINE_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_data_fill_color( svg, SC_DATA_FILL_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_data_line_color( svg, SC_DATA_LINE_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_x_gridline_color( svg, SC_XGRID_COLOR );
        if( rc == RC_NORMAL ) rc = svg_set_y_gridline_color( svg, SC_YGRID_COLOR );

        if( rc == RC_NORMAL ) rc = svg_set_graph_alpha( svg, SC_GRAPH_ALPHA );
        if( rc == RC_NORMAL ) rc = svg_set_circ_fill_alpha( svg, SC_CIRC_FILL_ALPHA );
        if( rc == RC_NORMAL ) rc = svg_set_circ_line_alpha( svg, SC_CIRC_LINE_ALPHA );
        if( rc == RC_NORMAL ) rc = svg_set_data_fill_alpha( svg, SC_DATA_FILL_ALPHA );
        if( rc == RC_NORMAL ) rc = svg_set_data_line_alpha( svg, SC_DATA_LINE_ALPHA );
    }

    if( rc == RC_NORMAL )
    {
        rc = svg_set_screen_width( svg, "100%" );
        if( rc == RC_NORMAL ) rc = svg_set_screen_height( svg, "50%" );

        if( rc == RC_NORMAL ) rc = svg_set_chart_title( svg, popt.chart_title );
        if( rc == RC_NORMAL ) rc = svg_set_xax_title( svg, popt.xax_title );
        if( rc == RC_NORMAL ) rc = svg_set_yax_title( svg, popt.yax_title );

        if( rc == RC_NORMAL && popt.xax_grids > 0 ) rc = svg_set_xax_num_grids( svg, popt.xax_grids );
        if( rc == RC_NORMAL && popt.yax_grids > 0 ) rc = svg_set_yax_num_grids( svg, popt.yax_grids );

        if( rc == RC_NORMAL ) rc = svg_finalize_model( svg );

        if( rc == RC_NORMAL)
        {
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
                if( rc == RC_NORMAL ) rc = svg_set_xax_disp( svg, dataformat );
                if( dataformat ) free( dataformat );
	    }
        }
    }

    if( rc == RC_NORMAL )
    {
        context = DO_RENDER_CHART;
        svg_doc = svg_render( &rc, svg );
    }

    if( rc == RC_NORMAL )
    {
        context = DO_OPEN_OUTPUT_FILE;

        if( *popt.out_file && strcmp(popt.out_file, IS_STDOUT) )
        {
            out = open( popt.out_file, OUT_OPEN_FLAGS, OUT_OPEN_MODE );
            if( out == -1 ) rc = ERR_OPEN_FAILED;
	}
        else out = fileno( stdout );
    }

    if( rc == RC_NORMAL )
    {
        context = DO_WRITE_SVG_DOC;
        svg_doc_len = strlen(svg_doc);
        nbyte = write( out, svg_doc, svg_doc_len );
	if( nbyte != svg_doc_len ) rc = ERR_WRITE_FAILED;
    }

    if( rc == RC_NORMAL ) svg_free_model( svg );

    /* --- */

    if( rc != RC_NORMAL )
    {
        fprintf( stderr, "Err: RC=%d, context: %s\n", rc, context_desc(context));
    }

    return( rc );
}
