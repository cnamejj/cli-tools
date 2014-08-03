#include <math.h>
#include "cli-sub.h"
#include "http-fetch.h"
#include "svg-tools.h"
#include "err_ref.h"

/* --- */

char *hf_generate_graph( int *rc, int cases, float *xdata, float *ydata, char *style, char *title,
  char *xax_title, char *yax_title, struct chart_options *chopt)

{
    int grids, digits, num_grids;
    float dmin, dmax, span;
    char *dataformat, *svg_doc = 0;
    struct svg_model *svg;
    struct milestone *walk, *mstone = 0;
    struct svg_chart_milestone *ckpt;

    if( *rc == RC_NORMAL)
    {
        svg = svg_make_chart();
        if( !svg) *rc = ERR_MALLOC_FAILED;
    }

    if( *rc == RC_NORMAL) *rc = svg_add_float_data( svg, cases, xdata, ydata);

    if( chopt )
    {
        mstone = chopt->mstone;

        for( walk = mstone; *rc == RC_NORMAL && walk; walk = walk->next)
        {
            ckpt = svg_add_xax_checkpoint( svg, walk->offset, walk->label);
            if( !ckpt) *rc = ERR_MALLOC_FAILED;
            else if( walk == mstone && strcmp( style, SVG_STYLE_DARK))
            {
                *rc = svg_set_checkpoint_line_color( ckpt, GR_MST_LINE_COLOR );
                if( *rc == RC_NORMAL) *rc = svg_set_checkpoint_text_color( ckpt, GR_MST_TEXT_COLOR );
            }
	}

        if( chopt->xmin_hard != CH_OPT_NO_VALUE ) (void) svg_set_xmin( svg, chopt->xmin_hard );
        else if( chopt->xmin_soft != CH_OPT_NO_VALUE && chopt->xmin_soft < svg->xmin ) (void) svg_set_xmin( svg, chopt->xmin_soft );

        if( chopt->xmax_hard != CH_OPT_NO_VALUE ) (void) svg_set_xmax( svg, chopt->xmax_hard );
        else if( chopt->xmax_soft != CH_OPT_NO_VALUE && chopt->xmax_soft > svg->xmax ) (void) svg_set_xmax( svg, chopt->xmax_soft );

        if( chopt->ymin_hard != CH_OPT_NO_VALUE ) (void) svg_set_ymin( svg, chopt->ymin_hard );
        else if( chopt->ymin_soft != CH_OPT_NO_VALUE && chopt->ymin_soft < svg->ymin ) (void) svg_set_ymin( svg, chopt->ymin_soft );

        if( chopt->ymax_hard != CH_OPT_NO_VALUE ) (void) svg_set_ymax( svg, chopt->ymax_hard );
        else if( chopt->ymax_soft != CH_OPT_NO_VALUE && chopt->ymax_soft > svg->ymax ) (void) svg_set_ymax( svg, chopt->ymax_soft );
    }


    if( *rc == RC_NORMAL) *rc = svg_set_chart_title( svg, title);
    if( *rc == RC_NORMAL) *rc = svg_set_xax_title( svg, xax_title);
    if( *rc == RC_NORMAL) *rc = svg_set_yax_title( svg, yax_title);
    if( *rc == RC_NORMAL)
    {
        if( cases < GR_ALL_XAX_GRIDS) num_grids = cases;
        else num_grids = GR_ALL_XAX_GRIDS;
        *rc = svg_set_xax_num_grids( svg, num_grids);
    }
    if( *rc == RC_NORMAL)
    {
        if( cases < GR_ALL_YAX_GRIDS) num_grids = cases;
        else num_grids = GR_ALL_YAX_GRIDS;
        *rc = svg_set_yax_num_grids( svg, num_grids);
    }

    if( *rc == RC_NORMAL) *rc = svg_finalize_model( svg);

    if( *rc == RC_NORMAL)
    {
        dmin = svg_get_xmin( svg);
        dmax = svg_get_xmax( svg);
        grids = svg_get_xax_num_grids( svg);
        span = (dmax - dmin) / grids;
        if( span >= 1.0) digits = 0;
        else if( span == 0.0) digits = 0;
        else digits = (int) (1 - log10( span));
/* printf( "<!-- HFGG: min:%f max:%f span:%f digits:%d patt(%s) -->\n", dmin, dmax, span, digits, GR_DATA_META_FORMAT); */
        dataformat = string_from_int( rc, digits, GR_DATA_META_FORMAT);
/* printf( "<!-- HFGG: rc:%d format(%s)-->\n", *rc, dataformat); */
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
        else if( span == 0.0) digits = 0;
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
    }

    if( *rc == RC_NORMAL && cases < GR_MIN_CASES)
    {
        result = strdup( GR_PACK_NOGRAPH);
        if( !result) *rc = ERR_MALLOC_FAILED;
    }

    else if( *rc == RC_NORMAL)
    {
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

        result = hf_generate_graph( rc, cases, elap, psize, style, title, GR_PACK_XAX_TITLE,
          GR_PACK_YAX_TITLE, 0);
    }

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
    char *result = 0, *title = 0, *mst_type = 0;
    float *elap = 0, *recbytes = 0, delta;
    struct ckpt_chain *stime, *walk;
    struct chart_options *chopt = 0;
    struct milestone *mstone = 0, *curr = 0, *last = 0;

    title = combine_strings( rc, GR_ACCDAT_TITLE_LEAD, url);

    if( *rc == RC_NORMAL)
    {
        if( ssl) event = EVENT_SSL_NET_READ;
        else event = EVENT_READ_PACKET;

        cases = 0;
        for( walk = fetch->checkpoint; walk; walk = walk->next)
          if( walk->event == event && walk->detail) cases++;
        cases++;
    }

    if( *rc == RC_NORMAL && cases < GR_MIN_CASES)
    {
        result = strdup( GR_PACK_NOGRAPH);
        if( !result) *rc = ERR_MALLOC_FAILED;
    }

    else if( *rc == RC_NORMAL)
    {
        elap = (float *) malloc( cases * (sizeof *elap));
        recbytes = (float *) malloc( cases * (sizeof *recbytes));

        if( !elap || !recbytes) *rc = ERR_MALLOC_FAILED;
        else
        {
            *elap = 0;
            *recbytes = 0;
            off = 1;
            stime = fetch->checkpoint;

            for( walk = fetch->checkpoint; walk; walk = walk->next)
            {
                if( walk->event == event && walk->detail)
                {
                    delta = calc_time_difference( &stime->clock, &walk->clock, fetch->clock_res);

                    *(elap + off) = delta;
                    accumulate += walk->detail->len;
                    *(recbytes + off) = accumulate;

                    off++;
		}
	    }

            chopt = alloc_chart_options();
            if( !chopt ) *rc = ERR_MALLOC_FAILED;
            else
            {
                for( walk = fetch->checkpoint; walk && *rc == RC_NORMAL; walk = walk->next)
                {
                    if( walk->event == EVENT_DNS_LOOKUP) mst_type = MSTONE_DNS;
                    else if( walk->event == EVENT_CONNECT_SERVER) mst_type = MSTONE_CONN;
                    else if( walk->event == EVENT_SSL_HANDSHAKE) mst_type = MSTONE_SSL;
                    else if( walk->event == EVENT_REQUEST_SENT) mst_type = MSTONE_SEND;
                    else if( walk->event == EVENT_FIRST_RESPONSE) mst_type = MSTONE_1STREAD;
                    else if( walk->event == EVENT_READ_ALL_DATA) mst_type = MSTONE_ALLDATA;
                    else mst_type = 0;

                    if( mst_type)
                    {
                        curr = (struct milestone *) malloc( sizeof *curr);
                        if( !curr) *rc = ERR_MALLOC_FAILED;
                        else
                        {
                            if( !mstone) chopt->mstone = mstone = curr;
                            if( last) last->next = curr;
                            curr->offset = calc_time_difference( &stime->clock, &walk->clock, fetch->clock_res);
                            curr->label = mst_type;
                            curr->next = 0;
                            last = curr;
			}
		    }
		}
	    }
	}

        result = hf_generate_graph( rc, cases, elap, recbytes, style, title, GR_ACCDAT_XAX_TITLE,
          GR_ACCDAT_YAX_TITLE, chopt);

        for( curr = mstone; curr; curr = last)
        {
            last = curr->next;
            free( curr);
	}
    }

    if( title) free( title);
    if( elap) free( elap);
    if( recbytes) free( recbytes);

    return( result);
}

/* --- */

char *make_psize_freq_graph( int *rc, char *url, char *style, int ssl, struct fetch_status *fetch)

{
    int event, dlen, dmin, dmax, off, cases, spot, heaps;
    char *result = 0, *title = 0;
    float *psfreq = 0, *bracket = 0, span;
    struct ckpt_chain *walk;
    struct chart_options *chopt = 0;

    heaps = GR_FREQ_BRACKETS;
    title = combine_strings( rc, GR_FR_PSIZE_TITLE_LEAD, url);

    if( *rc == RC_NORMAL)
    {
        if( ssl) event = EVENT_SSL_NET_READ;
        else event = EVENT_READ_PACKET;

        cases = 0;
        for( walk = fetch->checkpoint; walk; walk = walk->next)
        {
            if( walk->event == event && walk->detail)
            {
                dlen = walk->detail->len;
                if( !cases) dmin = dmax = dlen;
                if( dlen < dmin) dmin = dlen;
                if( dlen > dmax) dmax = dlen;
                cases++;
	    }
	}

/* printf( "dbg:: PSFR: cases:%d min:%d max:%d\n", cases, dmin, dmax); */

        bracket = (float *) malloc( heaps * (sizeof *bracket));
        psfreq = (float *) malloc( heaps * (sizeof *psfreq));

        if( !bracket || !psfreq) *rc = ERR_MALLOC_FAILED;
        else
        {
            for( off = 0; off < heaps; off++) *(psfreq + off) = 0.0;

            if( dmax == dmin)
            {
                *bracket = dmin;
                *psfreq = cases;
                heaps = 1;
	    }
            else
            {
                span = ((float) (dmax - dmin)) / heaps;
/* printf( "dbg:: PSFR: brackets:%d span:%f\n", heaps, span); */

                *bracket = dmin + (span / 2);
                for( off = 1; off < heaps; off++)
                  *(bracket + off) = *(bracket + off - 1) + span;

                for( walk = fetch->checkpoint; walk; walk = walk->next)
                {
                    if( walk->event == event && walk->detail)
                    {
                        dlen = walk->detail->len;
                        spot = (dlen - dmin) / span;
                        if( spot >= heaps ) spot = heaps - 1;
                        *(psfreq + spot) += 1;
		    }
		}

                spot = 0;
                for( off = 0; off < heaps; off++)
                {
                    if( *(psfreq + off) > 0.0)
                    {
                        *(psfreq + spot) = *(psfreq + off);
                        *(bracket + spot) = *(bracket + off);
                        spot++;
		    }
		}

                heaps = spot;
                if( !heaps) heaps = 1;

/*
for( off = 0; off < heaps; off++)
{
    printf( "dbg:: PSFR: data: %d. %f %f\n", off, *(bracket + off), *(psfreq + off));
}
 */
	    }
	}

        chopt = alloc_chart_options();
        if( !chopt ) *rc = ERR_MALLOC_FAILED;
        else chopt->ymin_soft = 0;

        result = hf_generate_graph( rc, heaps, bracket, psfreq, style, title, GR_FR_PSIZE_XAX_TITLE,
          GR_FR_PSIZE_YAX_TITLE, chopt);
    }

    if( title) free( title);
    if( psfreq) free( psfreq);
    if( bracket) free( bracket);

    return( result);
}

/* --- */

char *make_rwait_freq_graph( int *rc, char *url, char *style, int ssl, struct fetch_status *fetch)

{
    int event, off, cases, spot, heaps;
    char *result = 0, *title = 0;
    float *waitfreq = 0, *bracket = 0, span, delta, dmin, dmax;
    struct ckpt_chain *walk, *prev = 0;
    struct chart_options *chopt = 0;

    heaps = GR_FREQ_BRACKETS;
    title = combine_strings( rc, GR_FR_RWAIT_TITLE_LEAD, url);

    if( *rc == RC_NORMAL)
    {
        if( ssl) event = EVENT_SSL_NET_READ;
        else event = EVENT_READ_PACKET;

        cases = 0;
        prev = 0;
        for( walk = fetch->checkpoint; walk; walk = walk->next)
        {
            if( walk->event == event && walk->detail)
            {
                if( prev)
                {
                    delta = calc_time_difference( &prev->clock, &walk->clock, fetch->clock_res);
                    if( !cases) dmin = dmax = delta;
                    if( delta < dmin) dmin = delta;
                    if( delta > dmax) dmax = delta;
                    cases++;
		}
                prev = walk;
	    }
	}

/* printf( "<!-- dbg:: RWFR: cases:%d min:%f max:%f -->\n", cases, dmin, dmax); */

        bracket = (float *) malloc( heaps * (sizeof *bracket));
        waitfreq = (float *) malloc( heaps * (sizeof *waitfreq));

        if( !bracket || !waitfreq) *rc = ERR_MALLOC_FAILED;
        else
        {
            for( off = 0; off < heaps; off++) *(waitfreq + off) = 0.0;

            if( dmax == dmin)
            {
                *bracket = dmin;
                *waitfreq = cases;
                heaps = 1;
	    }
            else
            {
                span = (dmax - dmin) / heaps;
/* printf( "<!-- dbg:: RWFR: brackets:%d span:%f -->\n", heaps, span); */

                *bracket = dmin + (span / 2);

                for( off = 1; off < heaps; off++)
                  *(bracket + off) = *(bracket + off - 1) + span;

                prev = 0;
                for( walk = fetch->checkpoint; walk; walk = walk->next)
                {
                    if( walk->event == event && walk->detail)
                    {
                        if( prev)
                        {
                            delta = calc_time_difference( &prev->clock, &walk->clock, fetch->clock_res);
                            spot = (delta - dmin) / span;
                            if( spot >= heaps ) spot = heaps - 1;
                            *(waitfreq + spot) += 1;
                        }
                        prev = walk;
		    }
		}

                spot = 0;
                for( off = 0; off < heaps; off++)
                {
                    if( *(waitfreq + off) > 0.0)
                    {
                        *(waitfreq + spot) = *(waitfreq + off);
                        *(bracket + spot) = *(bracket + off);
                        spot++;
		    }
		}

                heaps = spot;
                if( !heaps) heaps = 1;
/*
for( off = 0; off < heaps; off++)
{
    printf( "!<-- dbg:: RWFR: data: %d. %f %f -->\n", off, *(bracket + off), *(waitfreq + off));
}
 */
	    }
	}

        chopt = alloc_chart_options();
        if( !chopt ) *rc = ERR_MALLOC_FAILED;
        else chopt->ymin_soft = 0;

        result = hf_generate_graph( rc, heaps, bracket, waitfreq, style, title, GR_FR_RWAIT_XAX_TITLE,
          GR_FR_RWAIT_YAX_TITLE, chopt);
    }

    if( title) free( title);
    if( waitfreq) free( waitfreq);
    if( bracket) free( bracket);

    return( result);
}

