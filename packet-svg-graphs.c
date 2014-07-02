#include <math.h>
#include "cli-sub.h"
#include "http-fetch.h"
#include "svg-tools.h"
#include "err_ref.h"

/* --- */

char *hf_generate_graph( int *rc, int cases, float *xdata, float *ydata, char *style, char *title, char *xax_title, char *yax_title)

{
    int grids, digits;
    float dmin, dmax, span;
    char *dataformat, *svg_doc = 0;
    struct svg_model *svg;

    if( *rc == RC_NORMAL)
    {
        svg = svg_make_chart();
        if( !svg) *rc = ERR_MALLOC_FAILED;
    }

    if( *rc == RC_NORMAL) *rc = svg_add_float_data( svg, cases, xdata, ydata);

    if( *rc == RC_NORMAL) *rc = svg_set_chart_title( svg, title);
    if( *rc == RC_NORMAL) *rc = svg_set_xax_title( svg, xax_title);
    if( *rc == RC_NORMAL) *rc = svg_set_yax_title( svg, yax_title);
    if( *rc == RC_NORMAL) *rc = svg_set_xax_num_grids( svg, GR_ALL_XAX_GRIDS);
    if( *rc == RC_NORMAL) *rc = svg_set_yax_num_grids( svg, GR_ALL_YAX_GRIDS);

    if( *rc == RC_NORMAL) *rc = svg_finalize_model( svg);

    if( *rc == RC_NORMAL)
    {
        dmin = svg_get_xmin( svg);
        dmax = svg_get_xmax( svg);
        grids = svg_get_xax_num_grids( svg);
        span = (dmax - dmin) / grids;
        if( span >= 1.0) digits = 0;
        else digits = (int) (1 - log10( span));
        dataformat = string_from_int( rc, digits, GR_DATA_META_FORMAT);
        if( *rc == RC_NORMAL) *rc = svg_set_xax_disp( svg, dataformat);
        if( dataformat) free( dataformat);
    }

    if( *rc == RC_NORMAL)
    {
        dmin = svg_get_ymin( svg);
        dmax = svg_get_ymax( svg);
        grids = svg_get_yax_num_grids( svg);
        span = (dmax - dmin) / grids;
        if( span >= 1.0) digits = 0;
        else digits = (int) (1 - log10( span));
        dataformat = string_from_int( rc, digits, GR_DATA_META_FORMAT);
        if( *rc == RC_NORMAL) *rc = svg_set_yax_disp( svg, dataformat);
        if( dataformat) free( dataformat);
    }

    if( strcmp( style, SVG_STYLE_DARK))
    {
        if( *rc == RC_NORMAL) *rc = svg_set_circ_line_size( svg, GR_ALL_CIRC_LINE_SIZE);
        if( *rc == RC_NORMAL) *rc = svg_set_circ_radius( svg, GR_ALL_CIRC_RADIUS);

        if( *rc == RC_NORMAL) *rc = svg_set_text_color( svg, GR_ALL_TEXT_COLOR);
        if( *rc == RC_NORMAL) *rc = svg_set_axis_color( svg, GR_ALL_AXIS_COLOR);
        if( *rc == RC_NORMAL) *rc = svg_set_chart_color( svg, GR_ALL_CHART_COLOR);
        if( *rc == RC_NORMAL) *rc = svg_set_graph_color( svg, GR_ALL_GRAPH_COLOR);
        if( *rc == RC_NORMAL) *rc = svg_set_circ_fill_color( svg, GR_ALL_CIRC_FILL_COLOR);
        if( *rc == RC_NORMAL) *rc = svg_set_circ_line_color( svg, GR_ALL_CIRC_LINE_COLOR);
        if( *rc == RC_NORMAL) *rc = svg_set_data_fill_color( svg, GR_ALL_DATA_FILL_COLOR);
        if( *rc == RC_NORMAL) *rc = svg_set_data_line_color( svg, GR_ALL_DATA_LINE_COLOR);
        if( *rc == RC_NORMAL) *rc = svg_set_x_gridline_color( svg, GR_ALL_XGRID_COLOR);
        if( *rc == RC_NORMAL) *rc = svg_set_y_gridline_color( svg, GR_ALL_YGRID_COLOR);

        if( *rc == RC_NORMAL) *rc = svg_set_graph_alpha( svg, GR_ALL_GRAPH_ALPHA);
        if( *rc == RC_NORMAL) *rc = svg_set_circ_fill_alpha( svg, GR_ALL_CIRC_FILL_ALPHA);
        if( *rc == RC_NORMAL) *rc = svg_set_circ_line_alpha( svg, GR_ALL_CIRC_LINE_ALPHA);
        if( *rc == RC_NORMAL) *rc = svg_set_data_fill_alpha( svg, GR_ALL_DATA_FILL_ALPHA);
        if( *rc == RC_NORMAL) *rc = svg_set_data_line_alpha( svg, GR_ALL_DATA_LINE_ALPHA);
    }

    if( *rc == RC_NORMAL) svg_doc = svg_render( rc, svg);

    if( svg) svg_free_model( svg);

    return( svg_doc);
}

/* --- */

char *make_packet_graph( int *rc, char *url, char *style, int ssl, struct fetch_status *fetch)

{
    int event, cases, off;
    char *result = 0, *title = 0;
    float *elap = 0, *psize = 0, delta;
    struct ckpt_chain *stime, *walk;

    title = combine_strings( rc, GR_PACK_TITLE_LEAD, url);

    if( *rc == RC_NORMAL)
    {
        if( ssl) event = EVENT_SSL_NET_READ;
        else event = EVENT_READ_PACKET;

        cases = 0;
        for( walk = fetch->checkpoint; walk; walk = walk->next)
          if( walk->event == event && walk->detail) cases++;

        elap = (float *) malloc( cases * (sizeof *elap));
        psize = (float *) malloc( cases * (sizeof *psize));

        if( !elap || !psize) *rc = ERR_MALLOC_FAILED;
        else
        {
            off = 0;
            stime = 0;

            for( walk = fetch->checkpoint; walk; walk = walk->next)
            {
                if( walk->event == event && walk->detail)
                {
                    if( !stime) stime = walk;
                    delta = calc_time_difference( &stime->clock, &walk->clock, fetch->clock_res);

                    *(elap + off) = delta;                    
                    *(psize + off) = walk->detail->len;

                    off++;
		}
	    }
	}
    }

    result = hf_generate_graph( rc, cases, elap, psize, style, title, GR_PACK_XAX_TITLE, GR_PACK_YAX_TITLE);

    if( title) free( title);
    if( elap) free( elap);
    if( psize) free( psize);

    return( result);
}

/* --- */

char *make_accdata_graph( int *rc, char *url, char *style, int ssl, struct fetch_status *fetch)

{
    int event, cases, off;
    long accumulate = 0;
    char *result = 0, *title = 0;
    float *elap = 0, *recbytes = 0, delta;
    struct ckpt_chain *stime, *walk;

    title = combine_strings( rc, GR_ACCDAT_TITLE_LEAD, url);

    if( *rc == RC_NORMAL)
    {
        if( ssl) event = EVENT_SSL_NET_READ;
        else event = EVENT_READ_PACKET;

        cases = 0;
        for( walk = fetch->checkpoint; walk; walk = walk->next)
          if( walk->event == event && walk->detail) cases++;

        elap = (float *) malloc( cases * (sizeof *elap));
        recbytes = (float *) malloc( cases * (sizeof *recbytes));

        if( !elap || !recbytes) *rc = ERR_MALLOC_FAILED;
        else
        {
            off = 0;
            stime = 0;

            for( walk = fetch->checkpoint; walk; walk = walk->next)
            {
                if( walk->event == event && walk->detail)
                {
                    if( !stime) stime = walk;
                    delta = calc_time_difference( &stime->clock, &walk->clock, fetch->clock_res);

                    *(elap + off) = delta;
                    accumulate += walk->detail->len;
                    *(recbytes + off) = accumulate;

                    off++;
		}
	    }
	}
    }

    result = hf_generate_graph( rc, cases, elap, recbytes, style, title, GR_ACCDAT_XAX_TITLE, GR_ACCDAT_YAX_TITLE);

    if( title) free( title);
    if( elap) free( elap);
    if( recbytes) free( recbytes);

    return( result);
}

/* --- */

char *make_psize_freq_graph( int *rc, char *url, struct fetch_status *fetch)

{
    char *svg_doc = 0;

    if( *rc == RC_NORMAL)
    {
        svg_doc = strdup( "<!-- packet-size freq graph goes here -->");
    }

    return( svg_doc);
}

/* --- */

char *make_rwait_freq_graph( int *rc, char *url, struct fetch_status *fetch)

{
    char *svg_doc = 0;

    if( *rc == RC_NORMAL)
    {
        svg_doc = strdup( "<!-- read-wait freq graph goes here -->");
    }

    return( svg_doc);
}

