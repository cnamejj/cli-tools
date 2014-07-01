#include <stdlib.h>
#include <string.h>
#include "svg-tools.h"
#include "cli-sub.h"
#include "err_ref.h"

#define ALPHA_DISP_FORMAT "%.4f"

#define ADD_SUB_PAIR_RULE( FROM, TO ) \
if( *rc == RC_NORMAL ) \
{ \
    pending = (struct sub_list *) malloc( sizeof *pending ); \
    if( !pending ) \
    { \
        if( rule ) rule->next = 0; \
        *rc = ERR_MALLOC_FAILED; \
    } \
    else \
    { \
        if( rule ) rule->next = pending; \
        rule = pending; \
        rule->next = 0; \
        rule->from = FROM ; \
        rule->to = TO ; \
    } \
}

#ifdef DEBUG_FREE
#define dbg_free( SPOT) \
{ \
   fprintf( stderr, "--> Calling free() from %s (%d), spot %x\n", __FILE__, \
     __LINE__, SPOT); \
   free( SPOT); \
}
#else
#define dbg_free( SPOT) free( SPOT)
#endif

/* --- */

struct svg_model *svg_make_chart()

{
    char *st = 0;
    struct svg_model *svg = 0;

    svg = (struct svg_model *) malloc( sizeof *svg );
    if( svg )
    {
        svg->cases = 0;

        svg->xmin = 0.0;
        svg->xmax = 0.0;
        svg->ymin = 0.0;
        svg->ymax = 0.0;

        svg->xdata = 0;
        svg->ydata = 0;

        svg->axis_size = SVG_NO_VALUE;
        svg->xax_num_grids = SVG_NO_VALUE;
        svg->yax_num_grids = SVG_NO_VALUE;
        svg->xax_border = SVG_NO_VALUE;
        svg->xax_text_floor = SVG_NO_VALUE;
        svg->xax_width = SVG_NO_VALUE;
        svg->xax_text_adj = SVG_NO_VALUE;
        svg->x_gridline_size = SVG_NO_VALUE;
        svg->yax_border = SVG_NO_VALUE;
        svg->yax_text_col = SVG_NO_VALUE;
        svg->yax_text_floor = SVG_NO_VALUE;
        svg->yax_height = SVG_NO_VALUE;
        svg->yax_text_adj = SVG_NO_VALUE;
        svg->y_gridline_size = SVG_NO_VALUE;
        svg->graph_left_col = SVG_NO_VALUE;
        svg->graph_width = SVG_NO_VALUE;
        svg->graph_height = SVG_NO_VALUE;
        svg->graph_top_row = SVG_NO_VALUE;
        svg->graph_bottom = SVG_NO_VALUE;
        svg->graph_height_midp = SVG_NO_VALUE;
        svg->graph_width_midp = SVG_NO_VALUE;
        svg->chart_height = DEF_CH_HI;
        svg->chart_width_midp = SVG_NO_VALUE;
        svg->chart_height_midp = SVG_NO_VALUE;
        svg->chart_width = DEF_CH_WID;
        svg->head_height_midp = SVG_NO_VALUE;
        svg->circ_line_size = SVG_NO_VALUE;
        svg->circ_radius = SVG_NO_VALUE;
        svg->data_line_size = SVG_NO_VALUE;
        svg->reserve_height = SVG_NO_VALUE;
        svg->reserve_width = SVG_NO_VALUE;
        svg->shift_width = SVG_NO_VALUE;
        svg->shift_height = SVG_NO_VALUE;
        svg->shift_bottom = SVG_NO_VALUE;

        svg->axis_alpha = DEF_AXIS_OP;
        svg->graph_alpha = DEF_GR_FILL_OP;
        svg->chart_alpha = DEF_BG_OP;
        svg->text_alpha = DEF_TEXT_OP;
        svg->circ_fill_alpha = DEF_CIR_FILL_OP;
        svg->circ_line_alpha = DEF_CIR_LIN_OP;
        svg->data_fill_alpha = DEF_DAT_FILL_OP;
        svg->data_line_alpha = DEF_DAT_LIN_OP;
        svg->x_gridline_alpha = DEF_X_GRID_OP;
        svg->y_gridline_alpha = DEF_Y_GRID_OP;

        svg->svt_row_label = 0;
        svg->svt_row_line = 0;
        svg->svt_col_label = 0;
        svg->svt_col_line = 0;
        svg->svt_circ_elem = 0;
        svg->svt_path_start = 0;
        svg->svt_path_points = 0;
        svg->svt_chart = 0;

        svg->chart_title = 0;
        svg->xax_title = 0;
        svg->yax_title = 0;
        svg->text_size = 0;
        svg->axis_color = 0;
        svg->chart_color = 0;
        svg->graph_color = 0;
        svg->circ_fill_color = 0;
        svg->circ_line_color = 0;
        svg->data_fill_color = 0;
        svg->data_line_color = 0;
        svg->text_color = 0;
        svg->x_gridline_color = 0;
        svg->y_gridline_color = 0;
        svg->xax_disp = 0;
        svg->yax_disp = 0;
        svg->screen_height = 0;
        svg->screen_width = 0;

        st = svg->chart_title = strdup( DEF_CHA_TITLE_TEXT );
        if( st ) st = svg->xax_title = strdup( DEF_XAXIS_TITLE );
        if( st ) st = svg->yax_title = strdup( DEF_YAXIS_TITLE );
        if( st ) st = svg->text_size = strdup( DEF_TEXT_SIZE );
        if( st ) st = svg->axis_color = strdup( DEF_AXIS_RGB );
        if( st ) st = svg->chart_color = strdup( DEF_BG_RGB );
        if( st ) st = svg->graph_color = strdup( DEF_GR_FILL_RGB );
        if( st ) st = svg->circ_fill_color = strdup( DEF_CIR_FILL_RGB );
        if( st ) st = svg->circ_line_color = strdup( DEF_CIR_LIN_RGB );
        if( st ) st = svg->data_fill_color = strdup( DEF_DAT_FILL_RGB );
        if( st ) st = svg->data_line_color = strdup( DEF_DAT_LIN_RGB );
        if( st ) st = svg->text_color = strdup( DEF_TEXT_RGB );
        if( st ) st = svg->x_gridline_color = strdup( DEF_X_GRID_RGB );
        if( st ) st = svg->y_gridline_color = strdup( DEF_Y_GRID_RGB );
        if( st ) st = svg->xax_disp = strdup( DEF_XAXIS_DIS_FORMAT );
        if( st ) st = svg->yax_disp = strdup( DEF_YAXIS_DIS_FORMAT );
        if( st ) st = svg->screen_height = strdup( DEF_SC_HI );
        if( st ) st = svg->screen_width = strdup( DEF_SC_WID );

        if( st ) st = svg->svt_row_label = strdup( SVG_ROW_LABEL );
        if( st ) st = svg->svt_row_line = strdup( SVG_ROW_LINE ); 
        if( st ) st = svg->svt_col_label = strdup( SVG_COL_LABEL );
        if( st ) st = svg->svt_col_line = strdup( SVG_COL_LINE );
        if( st ) st = svg->svt_circ_elem = strdup( SVG_CIRC_ELEM );
        if( st ) st = svg->svt_path_start = strdup( SVG_PATH_START );
        if( st ) st = svg->svt_path_points = strdup( SVG_PATH_POINTS );
        if( st ) st = svg->svt_chart = strdup( SVG_CHART_TEMPLATE );

        if( !st )
        {
            svg_free_model( svg );
            svg = 0;
	}
    }

    /* --- */

    return( svg );
}


/* --- */

void svg_free_model( struct svg_model *svg )

{
    if( svg )
    {
        if( svg->xdata ) dbg_free( svg->xdata );
        if( svg->ydata ) dbg_free( svg->ydata );

        svg->xdata = 0;
        svg->ydata = 0;

        if( svg->chart_title ) dbg_free( svg->chart_title );
        if( svg->xax_title ) dbg_free( svg->xax_title );
        if( svg->yax_title ) dbg_free( svg->yax_title );
        if( svg->text_size ) dbg_free( svg->text_size );
        if( svg->axis_color ) dbg_free( svg->axis_color );
        if( svg->chart_color ) dbg_free( svg->chart_color );
        if( svg->graph_color ) dbg_free( svg->graph_color );
        if( svg->circ_fill_color ) dbg_free( svg->circ_fill_color );
        if( svg->circ_line_color ) dbg_free( svg->circ_line_color );
        if( svg->data_fill_color ) dbg_free( svg->data_fill_color );
        if( svg->data_line_color ) dbg_free( svg->data_line_color );
        if( svg->text_color ) dbg_free( svg->text_color );
        if( svg->x_gridline_color ) dbg_free( svg->x_gridline_color );
        if( svg->y_gridline_color ) dbg_free( svg->y_gridline_color );
        if( svg->xax_disp ) dbg_free( svg->xax_disp );
        if( svg->yax_disp ) dbg_free( svg->yax_disp );
        if( svg->screen_height ) dbg_free( svg->screen_height );
        if( svg->screen_width ) dbg_free( svg->screen_width );
        if( svg->svt_row_label ) dbg_free( svg->svt_row_label );
        if( svg->svt_row_line ) dbg_free( svg->svt_row_line );
        if( svg->svt_col_label ) dbg_free( svg->svt_col_label );
        if( svg->svt_col_line ) dbg_free( svg->svt_col_line );
        if( svg->svt_circ_elem ) dbg_free( svg->svt_circ_elem );
        if( svg->svt_path_start ) dbg_free( svg->svt_path_start );
        if( svg->svt_path_points ) dbg_free( svg->svt_path_points );
        if( svg->svt_chart ) dbg_free( svg->svt_chart );

        svg->chart_title = 0;
        svg->xax_title = 0;
        svg->yax_title = 0;
        svg->text_size = 0;
        svg->axis_color = 0;
        svg->chart_color = 0;
        svg->graph_color = 0;
        svg->circ_fill_color = 0;
        svg->circ_line_color = 0;
        svg->data_fill_color = 0;
        svg->data_line_color = 0;
        svg->text_color = 0;
        svg->x_gridline_color = 0;
        svg->y_gridline_color = 0;
        svg->xax_disp = 0;
        svg->yax_disp = 0;
        svg->screen_height = 0;
        svg->screen_width = 0;
        svg->svt_row_label = 0;
        svg->svt_row_line = 0;
        svg->svt_col_label = 0;
        svg->svt_col_line = 0;
        svg->svt_circ_elem = 0;
        svg->svt_path_start = 0;
        svg->svt_path_points = 0;
        svg->svt_chart = 0;

        dbg_free( svg );
        svg = 0;
    }

    return;
}

/* --- */

int svg_finalize_model( struct svg_model *svg )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else
    {
        if( svg->xax_num_grids == SVG_NO_VALUE ) svg->xax_num_grids = svg->chart_height * 0.0055;
        if( svg->yax_num_grids == SVG_NO_VALUE ) svg->yax_num_grids = svg->chart_width * 0.00334;
        if( svg->reserve_height == SVG_NO_VALUE ) svg->reserve_height = svg->chart_height * 0.1;
        if( svg->reserve_width == SVG_NO_VALUE ) svg->reserve_width = svg->chart_width * 0.05;
        if( svg->shift_width == SVG_NO_VALUE ) svg->shift_width = svg->chart_width * 0.075;
        if( svg->shift_height == SVG_NO_VALUE ) svg->shift_height = svg->chart_height * 0.1;
        if( svg->shift_bottom == SVG_NO_VALUE ) svg->shift_bottom = svg->chart_height * 0.1;
        if( svg->chart_width_midp == SVG_NO_VALUE ) svg->chart_width_midp = svg->chart_width / 2;
        if( svg->chart_height_midp == SVG_NO_VALUE ) svg->chart_height_midp = svg->chart_height / 2;

        if( svg->data_line_size == SVG_NO_VALUE ) svg->data_line_size = svg->chart_height * 0.00334;
        if( svg->circ_line_size == SVG_NO_VALUE ) svg->circ_line_size = svg->chart_height * 0.00334;
        if( svg->circ_radius == SVG_NO_VALUE ) svg->circ_radius = svg->chart_height * 0.02;
        if( svg->x_gridline_size == SVG_NO_VALUE ) svg->x_gridline_size = svg->chart_height * 0.00334;
        if( svg->y_gridline_size == SVG_NO_VALUE ) svg->y_gridline_size = svg->chart_height * 0.00334;
        if( svg->xax_text_adj == SVG_NO_VALUE ) svg->xax_text_adj = svg->chart_height * 0.02;
        if( svg->yax_text_adj == SVG_NO_VALUE ) svg->yax_text_adj = svg->reserve_width * 0.15;
        if( svg->axis_size == SVG_NO_VALUE ) svg->axis_size = svg->chart_height * 0.00334;

        if( svg->graph_width == SVG_NO_VALUE ) svg->graph_width = svg->chart_width - (2 * svg->reserve_width) - svg->shift_width;
        if( svg->graph_height == SVG_NO_VALUE ) svg->graph_height = svg->chart_height - (2 * svg->reserve_height) - svg->shift_height - svg->shift_bottom;
        if( svg->yax_text_col == SVG_NO_VALUE ) svg->yax_text_col = svg->shift_width + svg->reserve_width - svg->yax_text_adj;
        if( svg->yax_text_floor == SVG_NO_VALUE ) svg->yax_text_floor = svg->graph_height + svg->shift_height + svg->reserve_height + svg->xax_text_adj;
        if( svg->yax_height == SVG_NO_VALUE ) svg->yax_height = svg->graph_height / svg->yax_num_grids;
        if( svg->xax_text_floor == SVG_NO_VALUE ) svg->xax_text_floor = svg->graph_height + svg->shift_height + svg->reserve_height + svg->xax_text_adj;
        if( svg->xax_width == SVG_NO_VALUE ) svg->xax_width = svg->graph_width / svg->xax_num_grids;
        if( svg->graph_left_col == SVG_NO_VALUE ) svg->graph_left_col = svg->reserve_width + svg->shift_width;
        if( svg->graph_top_row == SVG_NO_VALUE ) svg->graph_top_row = svg->reserve_height + svg->shift_height;
        if( svg->graph_bottom == SVG_NO_VALUE ) svg->graph_bottom = svg->chart_height - svg->shift_bottom;
        if( svg->head_height_midp == SVG_NO_VALUE ) svg->head_height_midp = (svg->shift_height + svg->reserve_height) / 2;
        if( svg->graph_height_midp == SVG_NO_VALUE ) svg->graph_height_midp = svg->shift_height + svg->reserve_height + (svg->graph_height / 2);
        if( svg->graph_width_midp == SVG_NO_VALUE ) svg->graph_width_midp = svg->graph_left_col + (svg->graph_width / 2 );
        if( svg->xax_border == SVG_NO_VALUE ) svg->xax_border = svg->graph_width + svg->reserve_width + svg->shift_width;
        if( svg->yax_border == SVG_NO_VALUE ) svg->yax_border = svg->graph_height + svg->reserve_height + svg->shift_height;

        if( svg->text_size ) if( !*svg->text_size ) svg->text_size = 0;
        if( !svg->text_size ) svg->text_size = string_from_int( &rc, svg->chart_height / 20, 0);
    }

    return( rc );
}

/* --- */

int svg_add_double_data( struct svg_model *svg, int cases, double *xval, double *yval)

{
    int rc = RC_NORMAL, off;
    double *xdata = 0, *ydata = 0, xmin, ymin, xmax, ymax, xd, yd;

    if( !svg || cases <= 0 ) rc = ERR_UNSUPPORTED;
    else
    {
        xdata = (double *) malloc( cases * (sizeof *xdata) );
        ydata = (double *) malloc( cases * (sizeof *ydata) );

        if( !xdata || !ydata )
        {
            rc = ERR_MALLOC_FAILED;
            if( xdata ) dbg_free( xdata );
            if( ydata ) dbg_free( ydata );
	}
        else
        {
            xmin = xmax = *xval;
            ymin = ymax = *yval;

            for( off=0; off<cases; off++)
            {
                xd = *(xval + off);
                yd = *(yval + off);
                *(xdata + off) = xd;
                *(ydata + off) = yd;
                if( xd < xmin ) xmin = xd;
                if( xd > xmax ) xmax = xd;
                if( yd < ymin ) ymin = yd;
                if( yd > ymax ) ymax = yd;
	    }

            svg->cases = cases;
            svg->xmin = xmin;
            svg->xmax = xmax;
            svg->ymin = ymin;
            svg->ymax = ymax;
            svg->xdata = xdata;
            svg->ydata = ydata;
	}
    }

    return( rc );
}

/* --- */

int svg_add_float_data( struct svg_model *svg, int cases, float *xval, float *yval)

{
    int rc = RC_NORMAL, off;
    double *xdata = 0, *ydata = 0, xmin, ymin, xmax, ymax, xd, yd;

    if( !svg || cases <= 0 ) rc = ERR_UNSUPPORTED;
    else
    {
        xdata = (double *) malloc( cases * (sizeof *xdata) );
        ydata = (double *) malloc( cases * (sizeof *ydata) );

        if( !xdata || !ydata )
        {
            rc = ERR_MALLOC_FAILED;
            if( xdata ) dbg_free( xdata );
            if( ydata ) dbg_free( ydata );
	}
        else
        {
            xmin = xmax = *xval;
            ymin = ymax = *yval;

            for( off=0; off<cases; off++)
            {
                xd = *(xval + off);
                yd = *(yval + off);
                *(xdata + off) = xd;
                *(ydata + off) = yd;
                if( xd < xmin ) xmin = xd;
                if( xd > xmax ) xmax = xd;
                if( yd < ymin ) ymin = yd;
                if( yd > ymax ) ymax = yd;
	    }

            svg->cases = cases;
            svg->xmin = xmin;
            svg->xmax = xmax;
            svg->ymin = ymin;
            svg->ymax = ymax;
            svg->xdata = xdata;
            svg->ydata = ydata;
	}
    }

    return( rc );
}

/* --- */

int svg_add_int_data( struct svg_model *svg, int cases, int *xval, int *yval)

{
    int rc = RC_NORMAL, off;
    double *xdata = 0, *ydata = 0, xmin, ymin, xmax, ymax, xd, yd;

    if( !svg || cases <= 0 ) rc = ERR_UNSUPPORTED;
    else
    {
        xdata = (double *) malloc( cases * (sizeof *xdata) );
        ydata = (double *) malloc( cases * (sizeof *ydata) );

        if( !xdata || !ydata )
        {
            rc = ERR_MALLOC_FAILED;
            if( xdata ) dbg_free( xdata );
            if( ydata ) dbg_free( ydata );
	}
        else
        {
            xmin = xmax = *xval;
            ymin = ymax = *yval;

            for( off=0; off<cases; off++)
            {
                xd = (double) *(xval + off);
                yd = (double) *(yval + off);
                *(xdata + off) = xd;
                *(ydata + off) = yd;
                if( xd < xmin ) xmin = xd;
                if( xd > xmax ) xmax = xd;
                if( yd < ymin ) ymin = yd;
                if( yd > ymax ) ymax = yd;
	    }

            svg->cases = cases;
            svg->xmin = xmin;
            svg->xmax = xmax;
            svg->ymin = ymin;
            svg->ymax = ymax;
            svg->xdata = xdata;
            svg->ydata = ydata;
	}
    }

    return( rc );
}

/* --- */

char *svg_make_yax_labels( int *rc, struct svg_model *svg )

{
    int point;
    char *svg_part = 0, *template, *one_label = 0, *accumulate = 0;
    float label, label_incr = 0.0;
    struct sub_list *subs = 0, *label_sub = 0, *walk = 0;

    if( *rc == RC_NORMAL && !svg ) *rc = ERR_UNSUPPORTED;

    if( *rc == RC_NORMAL )
    {
        label_incr = (svg->ymax - svg->ymin) / (float) svg->yax_num_grids;
        label = svg->ymin;

        template = svg->svt_row_label;

	subs = (struct sub_list *) malloc( sizeof *subs );
        if( !subs ) *rc = ERR_MALLOC_FAILED;
        else 
        {
            label_sub = subs;
            label_sub->from = S_LABEL;
            label_sub->to = 0;
            label_sub->next = 0;
	}
    }

    for( point = 1; *rc == RC_NORMAL && point <= svg->yax_num_grids; point++)
    {
        label += label_incr;

        if( label_sub->to ) dbg_free( label_sub->to );
        label_sub->to = string_from_float( rc, label, svg->yax_disp );
        if( !label_sub->to ) *rc = ERR_UNSUPPORTED;
        else
        {
            one_label = gsub_string( rc, template, subs );
            accumulate = combine_strings( rc, svg_part, one_label );

            if( one_label ) dbg_free( one_label );
            one_label = 0;

            if( svg_part ) dbg_free( svg_part );
            svg_part = accumulate;
            accumulate = 0;
	}
    }
    
    for( walk = subs; !walk; )
    {
        subs = walk->next;
        if( walk->to ) dbg_free( walk->to );
        dbg_free( walk );
        walk = subs;
    }

    /* --- */

    if( *rc != RC_NORMAL && svg_part )
    {
        dbg_free( svg_part );
        svg_part = 0;
    }

    return( svg_part );
}

/* --- */

char *svg_make_xax_labels( int *rc, struct svg_model *svg )

{
    int point, xpos;
    char *svg_part = 0, *template, *one_label = 0, *accumulate = 0;
    float label, label_incr = 0.0, xpos_incr = 0.0, xpos_adj = 0.0;
    struct sub_list *subs = 0, *label_sub = 0, *xpos_sub = 0, *walk = 0;

    if( *rc == RC_NORMAL && !svg ) *rc = ERR_UNSUPPORTED;

    if( *rc == RC_NORMAL )
    {
        label_incr = (svg->xmax - svg->xmin) / svg->xax_num_grids;
        label = svg->xmin;

        xpos_incr = (svg->xax_border - svg->graph_left_col) / (float) svg->xax_num_grids;
        xpos_adj = 0.0;

/* 
printf( "<!-- dbg:: xlabel: xax-border:%d gr-left-col:%d grids:%d incr:%f adj:%f -->\n",
  svg->xax_border, svg->graph_left_col, svg->xax_num_grids, xpos_incr, xpos_adj);
 */
  

        template = svg->svt_col_label;

	label_sub = (struct sub_list *) malloc( sizeof *label_sub );
        xpos_sub = (struct sub_list *) malloc( sizeof *xpos_sub );
        if( !label_sub || !xpos_sub ) *rc = ERR_MALLOC_FAILED;
        else 
        {
	    subs = label_sub;

            label_sub->from = S_LABEL;
            label_sub->to = 0;
            label_sub->next = xpos_sub;

            xpos_sub->from = S_XPOS;
            xpos_sub->to = 0;
            xpos_sub->next = 0;
	}
    }
    
    for( point = 1; *rc == RC_NORMAL && point <= svg->xax_num_grids; point++)
    {
        label += label_incr;
        xpos_adj += xpos_incr;
        xpos = svg->graph_left_col + (int) xpos_adj;

        if( label_sub->to ) dbg_free( label_sub->to );
        if( xpos_sub->to ) dbg_free( xpos_sub->to );

        label_sub->to = string_from_float( rc, label, svg->xax_disp );
        xpos_sub->to = string_from_int( rc, xpos, 0 );

        if( !label_sub->to || !xpos_sub->to ) *rc = ERR_UNSUPPORTED;
        else
        {
            one_label = gsub_string( rc, template, subs );
            accumulate = combine_strings( rc, svg_part, one_label );

            if( one_label ) dbg_free( one_label );
            one_label = 0;

            if( svg_part ) dbg_free( svg_part );
            svg_part = accumulate;
            accumulate = 0;
	}
    }
    
    for( walk = subs; !walk; )
    {
        subs = walk->next;
        if( walk->to ) dbg_free( walk->to );
        dbg_free( walk );
        walk = subs;
    }

    /* --- */

    if( *rc != RC_NORMAL && svg_part )
    {
        dbg_free( svg_part );
        svg_part = 0;
    }

    return( svg_part );
}

/* --- */

char *svg_make_yax_grid( int *rc, struct svg_model *svg )

{
    int pt;
    char *svg_part = 0, *template;
 
    if( *rc == RC_NORMAL && !svg ) *rc = ERR_UNSUPPORTED;

    if( *rc == RC_NORMAL )
    {
        template = svg->svt_row_line;

        svg_part = (char *) malloc( svg->yax_num_grids * strlen( template ) + 1 );
        if( !svg_part ) *rc = ERR_MALLOC_FAILED;
        else
        {
            *svg_part = '\0';
            for( pt = 1; pt <= svg->yax_num_grids; pt++) strcat( svg_part, template );
	}
    }
    
    /* --- */

    if( *rc != RC_NORMAL && svg_part )
    {
        dbg_free( svg_part );
        svg_part = 0;
    }

    return( svg_part );
}

/* --- */

char *svg_make_xax_grid( int *rc, struct svg_model *svg )

{
    int pt;
    char *svg_part = 0, *template;
 
    if( *rc == RC_NORMAL && !svg ) *rc = ERR_UNSUPPORTED;

    if( *rc == RC_NORMAL )
    {
        template = svg->svt_col_line;

        svg_part = (char *) malloc( svg->xax_num_grids * strlen( template ) + 1 );
        if( !svg_part ) *rc = ERR_MALLOC_FAILED;
        else
        {
            *svg_part = '\0';
            for( pt = 1; pt <= svg->xax_num_grids; pt++) strcat( svg_part, template );
	}
    }
    
    /* --- */

    if( *rc != RC_NORMAL && svg_part )
    {
        dbg_free( svg_part );
        svg_part = 0;
    }

    return( svg_part );
}

/* --- */

char *svg_make_data_points( int *rc, struct svg_model *svg )

{
    int xpos, ypos, xbase, ybase, xscale, yscale, pt;
    float xval, yval, xpc, ypc;
    double *xdata, *ydata;
    char *spots = 0, *template = 0, *acc = 0, *one_line = 0;
    struct sub_list *xpos_sub, *ypos_sub, *subs = 0, *walk = 0;

    if( *rc == RC_NORMAL )
    {
        template = svg->svt_circ_elem;

        xbase = svg->graph_left_col;
        xscale = svg->xax_border - svg->graph_left_col;
 
        ybase = svg->yax_border;
        yscale = svg->yax_border - svg->graph_top_row;
 
        xpos_sub = (struct sub_list *) malloc( sizeof *xpos_sub );
        ypos_sub = (struct sub_list *) malloc( sizeof *ypos_sub );

        if( !xpos_sub || !ypos_sub ) *rc = ERR_MALLOC_FAILED;
        else
        {
            subs = xpos_sub;

            xpos_sub->from = S_XPOS;
            xpos_sub->to = 0;
            xpos_sub->next = ypos_sub;

            ypos_sub->from = S_YPOS;
            ypos_sub->to = 0;
            ypos_sub->next = 0;
	}

        xdata = svg->xdata;
        ydata = svg->ydata;

        for( pt = 0; pt < svg->cases && *rc == RC_NORMAL; pt++ )
        {
            xval = *(xdata + pt);
            xpc = (xval - svg->xmin) / (svg->xmax - svg->xmin);
            xpos = xbase + (xscale * xpc);

            yval = *(ydata + pt);
            ypc = (yval - svg->ymin) / (svg->ymax - svg->ymin);
            ypos = ybase - (yscale * ypc);

            if( xpos_sub->to ) free( xpos_sub->to );
            if( ypos_sub->to ) free( ypos_sub->to );

            xpos_sub->to = string_from_int( rc, xpos, 0 );
            ypos_sub->to = string_from_int( rc, ypos, 0 );

            one_line = gsub_string( rc, template, subs );
            acc = combine_strings( rc, spots, one_line );

            if( one_line ) free( one_line );
            if( spots ) free( spots );

            spots = acc;
            acc = 0;
            one_line = 0;
	}
    }

    for( walk = subs; walk; )
    {
        if( walk->to ) free( walk->to );
        subs = walk->next;
        free( walk );
        walk = subs;
    }

    if( *rc != RC_NORMAL && spots )
    {
        free( spots );
        spots = 0;
    }

    return( spots );
}

/* --- */

char *svg_make_path_start( int *rc, struct svg_model *svg )

{
    int xpos, ypos, xscale, yscale;
    float xpc, ypc;
    char *pstart = 0, *template = 0;
    struct sub_list *xpos_sub, *ypos_sub, *subs = 0, *walk = 0;

    if( *rc == RC_NORMAL )
    {
        template = svg->svt_path_start;

        xscale = svg->xax_border - svg->graph_left_col;
        xpc = (*svg->xdata - svg->xmin) / (svg->xmax - svg->xmin);
        xpos = svg->graph_left_col + (xscale * xpc);

        yscale = svg->yax_border - svg->graph_top_row;
/*        ypc = (svg->ymax - *svg->ydata) / (svg->ymax - svg->ymin); */
        ypc = (*svg->ydata - svg->ymin) / (svg->ymax - svg->ymin); 
        ypos = svg->graph_top_row + (yscale * ypc); 
        ypos = svg->yax_border - (yscale * ypc); 

        xpos_sub = (struct sub_list *) malloc( sizeof *xpos_sub );
        ypos_sub = (struct sub_list *) malloc( sizeof *ypos_sub );

        if( !xpos_sub || !ypos_sub ) *rc = ERR_MALLOC_FAILED;
        else
        {
            subs = xpos_sub;

            xpos_sub->from = S_XPOS;
            xpos_sub->to = string_from_int( rc, xpos, 0 );
            xpos_sub->next = ypos_sub;

            ypos_sub->from = S_YPOS;
            ypos_sub->to = string_from_int( rc, ypos, 0 );
            ypos_sub->next = 0;
	}

        if( *rc == RC_NORMAL ) pstart = gsub_string( rc, template, subs );
    }

    for( walk = subs; walk; )
    {
        if( walk->to ) free( walk->to );
        subs = walk->next;
        free( walk );
        walk = subs;
    }

    if( *rc != RC_NORMAL && pstart )
    { 
        free( pstart );
        pstart = 0;
    }

    return( pstart );
}

/* --- */

char *svg_make_data_lines( int *rc, struct svg_model *svg )

{
    int xpos, ypos, xbase, ybase, xscale, yscale, pt;
    float xval, yval, xpc, ypc;
    double *xdata, *ydata;
    char *lines = 0, *template = 0, *acc = 0, *one_line = 0;
    struct sub_list *xpos_sub, *ypos_sub, *subs = 0, *walk = 0;

    if( *rc == RC_NORMAL )
    {
        template = svg->svt_path_points;

        xbase = svg->graph_left_col;
        xscale = svg->xax_border - svg->graph_left_col;
 
        ybase = svg->yax_border;
        yscale = svg->yax_border - svg->graph_top_row;
 
        xpos_sub = (struct sub_list *) malloc( sizeof *xpos_sub );
        ypos_sub = (struct sub_list *) malloc( sizeof *ypos_sub );

        if( !xpos_sub || !ypos_sub ) *rc = ERR_MALLOC_FAILED;
        else
        {
            subs = xpos_sub;

            xpos_sub->from = S_XPOS;
            xpos_sub->to = 0;
            xpos_sub->next = ypos_sub;

            ypos_sub->from = S_YPOS;
            ypos_sub->to = 0;
            ypos_sub->next = 0;
	}

        xdata = svg->xdata;
        ydata = svg->ydata;

        for( pt = 0; pt < svg->cases && *rc == RC_NORMAL; pt++ )
        {
            xval = *(xdata + pt);
            xpc = (xval - svg->xmin) / (svg->xmax - svg->xmin);
            xpos = xbase + (xscale * xpc);

            yval = *(ydata + pt);
            ypc = (yval - svg->ymin) / (svg->ymax - svg->ymin);
            ypos = ybase - (yscale * ypc);

            if( xpos_sub->to ) free( xpos_sub->to );
            if( ypos_sub->to ) free( ypos_sub->to );

            xpos_sub->to = string_from_int( rc, xpos, 0 );
            ypos_sub->to = string_from_int( rc, ypos, 0 );

            one_line = gsub_string( rc, template, subs );
            acc = combine_strings( rc, lines, one_line );

            if( one_line ) free( one_line );
            if( lines ) free( lines );

            lines = acc;
            acc = 0;
            one_line = 0;
	}
    }

    for( walk = subs; walk; )
    {
        if( walk->to ) free( walk->to );
        subs = walk->next;
        free( walk );
        walk = subs;
    }

    if( *rc != RC_NORMAL && lines )
    {
        free( lines );
        lines = 0;
    }

    return( lines );
}

/* --- */

char *svg_render( int *rc, struct svg_model *svg )

{
    char *chart = 0, *template = 0, *xax_label = 0, *yax_label = 0,
      *xax_grid = 0, *yax_grid = 0, *data_points = 0, *path_start = 0,
      *data_lines = 0;
    struct sub_list *subs = 0, *walk = 0, *rule = 0, *pending = 0;

    if( *rc == RC_NORMAL ) *rc = svg_finalize_model( svg );

    if( *rc == RC_NORMAL ) subs = svg_make_sublist( rc, svg );

    if( *rc == RC_NORMAL )
    {
        template = svg_make_xax_labels( rc, svg );
        if( *rc == RC_NORMAL) xax_label = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    if( *rc == RC_NORMAL )
    {
        template = svg_make_yax_labels( rc, svg );
        if( *rc == RC_NORMAL) yax_label = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    if( *rc == RC_NORMAL )
    {
        template = svg_make_xax_grid( rc, svg );
        if( *rc == RC_NORMAL) xax_grid = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    if( *rc == RC_NORMAL )
    {
        template = svg_make_yax_grid( rc, svg );
        if( *rc == RC_NORMAL) yax_grid = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    if( *rc == RC_NORMAL )
    {
        template = svg_make_data_points( rc, svg );
        if( *rc == RC_NORMAL) data_points = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    if( *rc == RC_NORMAL )
    {
        template = svg_make_path_start( rc, svg );
        if( *rc == RC_NORMAL) path_start = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    if( *rc == RC_NORMAL )
    {
        template = svg_make_data_lines( rc, svg );
        if( *rc == RC_NORMAL) data_lines = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    if( *rc == RC_NORMAL )
    {
        for( rule = subs; rule->next; rule = rule->next) ;
        ADD_SUB_PAIR_RULE( S_ST_COL_LABEL, xax_label )
        ADD_SUB_PAIR_RULE( S_ST_ROW_LABEL, yax_label )
        ADD_SUB_PAIR_RULE( S_ST_COL_LINE, xax_grid )
        ADD_SUB_PAIR_RULE( S_ST_ROW_LINE, yax_grid )
        ADD_SUB_PAIR_RULE( S_ST_DATA_POINT, data_points )
        ADD_SUB_PAIR_RULE( S_ST_START_PATH, path_start )
        ADD_SUB_PAIR_RULE( S_ST_DATA_LINE, data_lines )
    }

    if( *rc == RC_NORMAL )
    {
        template = svg->svt_chart;

        chart = gsub_string( rc, template, subs );
    }

    for( walk = subs; walk; )
    {
        if( walk->to ) free( walk->to );
        subs = walk->next;
        free( walk );
        walk = subs;
    }

    if( *rc != RC_NORMAL && chart )
    {
        free( chart );
        chart = 0;
    }

    return( chart );
}

/* --- */

struct sub_list *svg_make_sublist( int *rc, struct svg_model *svg )

{
    struct sub_list *allsubs = 0, *rule = 0, *pending = 0;

    if( *rc == RC_NORMAL )
    {
        ADD_SUB_PAIR_RULE( S_AXIS_RGB, svg->axis_color )
        allsubs = rule;
    }

    ADD_SUB_PAIR_RULE( S_AXIS_OP, string_from_float( rc, svg->axis_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_AXIS_SIZE, string_from_int( rc, svg->axis_size, 0 ) )
    ADD_SUB_PAIR_RULE( S_BG_RGB, svg->chart_color )
    ADD_SUB_PAIR_RULE( S_CH_HI, string_from_int( rc, svg->chart_height, 0 ) )
    ADD_SUB_PAIR_RULE( S_CH_WID, string_from_int( rc, svg->chart_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_CIR_FILL_RGB, svg->circ_fill_color )
    ADD_SUB_PAIR_RULE( S_CIR_FILL_OP, string_from_float( rc, svg->circ_fill_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_CIR_LIN_RGB, svg->circ_line_color )
    ADD_SUB_PAIR_RULE( S_CIR_LIN_OP, string_from_float( rc, svg->circ_line_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_CIR_LIN_SIZE, string_from_int( rc, svg->circ_line_size, 0 ) )
    ADD_SUB_PAIR_RULE( S_CIR_RAD, string_from_int( rc, svg->circ_radius, 0 ) )
    ADD_SUB_PAIR_RULE( S_DAT_FILL_RGB, svg->data_fill_color )
    ADD_SUB_PAIR_RULE( S_DAT_FILL_OP, string_from_float( rc, svg->data_fill_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_DAT_LIN_RGB, svg->data_line_color )
    ADD_SUB_PAIR_RULE( S_DAT_LIN_OP, string_from_float( rc, svg->data_line_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_DAT_LIN_SIZE, string_from_int( rc, svg->data_line_size, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_FILL_RGB, svg->graph_color )
    ADD_SUB_PAIR_RULE( S_GR_FILL_OP, string_from_float( rc, svg->graph_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_GR_AREA_HI, string_from_int( rc, svg->graph_height, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_AREA_WID, string_from_int( rc, svg->graph_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_LEFT_COL, string_from_int( rc, svg->graph_left_col, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_TOP_ROW, string_from_int( rc, svg->graph_top_row, 0 ) )
    ADD_SUB_PAIR_RULE( S_HI_RES, string_from_int( rc, svg->reserve_height, 0 ) )
    ADD_SUB_PAIR_RULE( S_SC_HI, svg->screen_height )
    ADD_SUB_PAIR_RULE( S_SC_WID, svg->screen_width )
    ADD_SUB_PAIR_RULE( S_TEXT_RGB, svg->text_color )
    ADD_SUB_PAIR_RULE( S_TEXT_OP, string_from_float( rc, svg->text_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_TEXT_SIZE, svg->text_size )
    ADD_SUB_PAIR_RULE( S_WID_RES, string_from_int( rc, svg->reserve_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_XAXIS_WID, string_from_int( rc, svg->xax_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_X_CH_BORD, string_from_int( rc, svg->xax_border, 0 ) )
    ADD_SUB_PAIR_RULE( S_X_GRID_RGB, svg->x_gridline_color )
    ADD_SUB_PAIR_RULE( S_X_GRID_OP, string_from_float( rc, svg->x_gridline_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_X_GRID_SIZE, string_from_int( rc, svg->x_gridline_size, 0 ) )
    ADD_SUB_PAIR_RULE( S_YAXIS_HI, string_from_int( rc, svg->yax_height, 0 ) )
    ADD_SUB_PAIR_RULE( S_YAXIS_TEXT_COL, string_from_int( rc, svg->yax_text_col, 0 ) )
    ADD_SUB_PAIR_RULE( S_YAXIS_TEXT_FLOOR, string_from_int( rc, svg->yax_text_floor, 0 ) )
    ADD_SUB_PAIR_RULE( S_YAXIS_TITLE, svg->yax_title )
    ADD_SUB_PAIR_RULE( S_XAXIS_TEXT_FLOOR, string_from_int( rc, svg->xax_text_floor, 0 ) )
    ADD_SUB_PAIR_RULE( S_XAXIS_TITLE, svg->xax_title )
    ADD_SUB_PAIR_RULE( S_Y_CH_BORD, string_from_int( rc, svg->yax_border, 0 ) )
    ADD_SUB_PAIR_RULE( S_Y_GRID_RGB, svg->y_gridline_color )
    ADD_SUB_PAIR_RULE( S_Y_GRID_OP, string_from_float( rc, svg->y_gridline_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_Y_GRID_SIZE, string_from_int( rc, svg->y_gridline_size, 0 ) )
    ADD_SUB_PAIR_RULE( S_CHA_TITLE_TEXT, svg->chart_title )
    ADD_SUB_PAIR_RULE( S_CHA_WID_MIDPOINT, string_from_int( rc, svg->chart_width_midp, 0 ) )
    ADD_SUB_PAIR_RULE( S_CHA_HI_MIDPOINT, string_from_int( rc, svg->chart_height_midp, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_WID_MIDPOINT, string_from_int( rc, svg->graph_width_midp, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_HI_MIDPOINT, string_from_int( rc, svg->graph_height_midp, 0 ) )
    ADD_SUB_PAIR_RULE( S_HDR_HI_MIDPOINT, string_from_int( rc, svg->head_height_midp, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_BOTTOM, string_from_int( rc, svg->graph_bottom, 0 ) )

    if( *rc != RC_NORMAL && allsubs )
    {
        for( rule = allsubs; rule; )
        {
            if( rule->to ) free( rule->to );
            allsubs = rule->next;
            free( rule );
            rule = allsubs;
	}

        allsubs = 0;
    }
/*
{
  int dbg = 0;
  for( rule = allsubs; rule; rule = rule->next )
  {
    printf( "<!-- dbg:: Sub: %d. (%s) -> (%s) -->\n", ++dbg, rule->from, rule->to);
  }
}
 */

    return( allsubs );
}

/* --- */

int svg_replace_string( char **current, char *replacement )

{
    int rc = RC_NORMAL;

    if( !current ) rc = ERR_BAD_PARMS;
    else
    {
        if( *current ) free( *current );
        *current = strdup( replacement );
        if( !*current ) rc = ERR_MALLOC_FAILED;
    }

    return( rc );
}

/* --- */

int svg_set_chart_title( struct svg_model *svg, char *title )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !title ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->chart_title, title );

    return( rc );
}

/* --- */

char *svg_get_chart_title( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->chart_title );
}

/* --- */

int svg_set_xax_title( struct svg_model *svg, char *title )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !title ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->xax_title, title );

    return( rc );
}

/* --- */

char *svg_get_xax_title( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->xax_title );
}

/* --- */

int svg_set_yax_title( struct svg_model *svg, char *title )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !title ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->yax_title, title );

    return( rc );
}

/* --- */

char *svg_get_yax_title( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->yax_title );
}

/* --- */

int svg_set_text_size( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->text_size, val );

    return( rc );
}

/* --- */

char *svg_get_text_size( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->text_size );
}

/* --- */

int svg_set_text_color( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->text_color, val );

    return( rc );
}

/* --- */

char *svg_get_text_color( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->text_color );
}


/* --- */

int svg_set_axis_color( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->axis_color, val );

    return( rc );
}

/* --- */

char *svg_get_axis_color( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->axis_color );
}

/* --- */

int svg_set_chart_color( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->chart_color, val );

    return( rc );
}

/* --- */

char *svg_get_chart_color( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->chart_color );
}

/* --- */

int svg_set_graph_color( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->graph_color, val );

    return( rc );
}

/* --- */

char *svg_get_graph_color( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->graph_color );
}

/* --- */

int svg_set_circ_fill_color( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->circ_fill_color, val );

    return( rc );
}

/* --- */

char *svg_get_circ_fill_color( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->circ_fill_color );
}

/* --- */

int svg_set_circ_line_color( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->circ_line_color, val );

    return( rc );
}

/* --- */

char *svg_get_circ_line_color( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->circ_line_color );
}

/* --- */

int svg_set_data_fill_color( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->data_fill_color, val );

    return( rc );
}

/* --- */

char *svg_get_data_fill_color( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->data_fill_color );
}

/* --- */

int svg_set_data_line_color( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->data_line_color, val );

    return( rc );
}

/* --- */

char *svg_get_data_line_color( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->data_line_color );
}

/* --- */

int svg_set_x_gridline_color( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->x_gridline_color, val );

    return( rc );
}

/* --- */

char *svg_get_x_gridline_color( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->x_gridline_color );
}

/* --- */

int svg_set_y_gridline_color( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->y_gridline_color, val );

    return( rc );
}

/* --- */

char *svg_get_y_gridline_color( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->y_gridline_color );
}

/* --- */

int svg_set_xax_disp( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->xax_disp, val );

    return( rc );
}

/* --- */

char *svg_get_xax_disp( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->xax_disp );
}

/* --- */

int svg_set_yax_disp( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->yax_disp, val );

    return( rc );
}

/* --- */

char *svg_get_yax_disp( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->yax_disp );
}

/* --- */

int svg_set_xax_num_grids( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->xax_num_grids = size;

    return( rc );
}

/* --- */

int svg_get_xax_num_grids( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->xax_num_grids );
}

/* --- */

int svg_set_yax_num_grids( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->yax_num_grids = size;

    return( rc );
}

/* --- */

int svg_get_yax_num_grids( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->yax_num_grids );
}

/* --- */

int svg_set_reserve_height( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->reserve_height = size;

    return( rc );
}

/* --- */

int svg_get_reserve_height( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->reserve_height );
}

/* --- */

int svg_set_reserve_width( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->reserve_width = size;

    return( rc );
}

/* --- */

int svg_get_reserve_width( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->reserve_width );
}

/* --- */

int svg_set_shift_height( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->shift_height = size;

    return( rc );
}

/* --- */

int svg_get_shift_height( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->shift_height );
}

/* --- */

int svg_set_shift_width( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->shift_width = size;

    return( rc );
}

/* --- */

int svg_get_shift_width( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->shift_width );
}

/* --- */

int svg_set_shift_bottom( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->shift_bottom = size;

    return( rc );
}

/* --- */

int svg_get_shift_bottom( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->shift_bottom );
}

/* --- */

int svg_set_data_line_size( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->data_line_size = size;

    return( rc );
}

/* --- */

int svg_get_data_line_size( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->data_line_size );
}

/* --- */

int svg_set_circ_line_size( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->circ_line_size = size;

    return( rc );
}

/* --- */

int svg_get_circ_line_size( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->circ_line_size );
}

/* --- */

int svg_set_circ_radius( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->circ_radius = size;

    return( rc );
}

/* --- */

int svg_get_circ_radius( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->circ_radius );
}

/* --- */

int svg_set_x_gridline_size( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->x_gridline_size = size;

    return( rc );
}

/* --- */

int svg_get_x_gridline_size( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->x_gridline_size );
}

/* --- */

int svg_set_y_gridline_size( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->y_gridline_size = size;

    return( rc );
}

/* --- */

int svg_get_y_gridline_size( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->y_gridline_size );
}

/* --- */

int svg_set_xax_text_adj( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->xax_text_adj = size;

    return( rc );
}

/* --- */

int svg_get_xax_text_adj( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->xax_text_adj );
}

/* --- */

int svg_set_yax_text_adj( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->yax_text_adj = size;

    return( rc );
}

/* --- */

int svg_get_yax_text_adj( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->yax_text_adj );
}

/* --- */

int svg_set_axis_size( struct svg_model *svg, int size )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->axis_size = size;

    return( rc );
}

/* --- */

int svg_get_axis_size( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->axis_size );
}

/* --- */

int svg_set_axis_alpha( struct svg_model *svg, float val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->axis_alpha = val;

    return( rc );
}

/* --- */

float svg_get_axis_alpha( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->axis_alpha );
}

/* --- */

int svg_set_graph_alpha( struct svg_model *svg, float val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->graph_alpha = val;

    return( rc );
}

/* --- */

float svg_get_graph_alpha( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->graph_alpha );
}

/* --- */

int svg_set_chart_alpha( struct svg_model *svg, float val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->chart_alpha = val;

    return( rc );
}

/* --- */

float svg_get_chart_alpha( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->chart_alpha );
}

/* --- */

int svg_set_text_alpha( struct svg_model *svg, float val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->text_alpha = val;

    return( rc );
}

/* --- */

float svg_get_text_alpha( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->text_alpha );
}

/* --- */

int svg_set_circ_fill_alpha( struct svg_model *svg, float val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->circ_fill_alpha = val;

    return( rc );
}

/* --- */

float svg_get_circ_fill_alpha( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->circ_fill_alpha );
}

/* --- */

int svg_set_circ_line_alpha( struct svg_model *svg, float val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->circ_line_alpha = val;

    return( rc );
}

/* --- */

float svg_get_circ_line_alpha( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->circ_line_alpha );
}

/* --- */

int svg_set_data_fill_alpha( struct svg_model *svg, float val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->data_fill_alpha = val;

    return( rc );
}

/* --- */

float svg_get_data_fill_alpha( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->data_fill_alpha );
}

/* --- */

int svg_set_data_line_alpha( struct svg_model *svg, float val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->data_line_alpha = val;

    return( rc );
}

/* --- */

float svg_get_data_line_alpha( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->data_line_alpha );
}

/* --- */

int svg_set_x_gridline_alpha( struct svg_model *svg, float val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->x_gridline_alpha = val;

    return( rc );
}

/* --- */

float svg_get_x_gridline_alpha( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->x_gridline_alpha );
}

/* --- */

int svg_set_y_gridline_alpha( struct svg_model *svg, float val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->y_gridline_alpha = val;

    return( rc );
}

/* --- */

float svg_get_y_gridline_alpha( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->y_gridline_alpha );
}

/* --- */

int svg_set_xmin( struct svg_model *svg, double val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->xmin = val;

    return( rc );
}

/* --- */

double svg_get_xmin( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->xmin );
}


/* --- */

int svg_set_xmax( struct svg_model *svg, double val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->xmax = val;

    return( rc );
}

/* --- */

double svg_get_xmax( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->xmax );
}


/* --- */

int svg_set_ymin( struct svg_model *svg, double val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->ymin = val;

    return( rc );
}

/* --- */

double svg_get_ymin( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->ymin );
}


/* --- */

int svg_set_ymax( struct svg_model *svg, double val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->ymax = val;

    return( rc );
}

/* --- */

double svg_get_ymax( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->ymax );
}
