#include <stdlib.h>
#include <string.h>
#include "svg-tools.h"
#include "cli-sub.h"
#include "err_ref.h"

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

/* --- */

struct series_data *get_empty_data_series( struct svg_model *svg )

{
    struct series_data *slot = 0, *ds = 0;
    
    if( !svg ) return( slot );

    for( ds = svg->series; !slot && ds; ds = ds->next )
    {
        if( ds->id == SVG_EMPTY_SERIES ) slot = ds;
    }

    if( !slot ) slot = add_data_series( svg );

    return( slot );
}

/* --- */

struct series_data *add_data_series( struct svg_model *svg )

{
    char *st = 0;
    struct series_data *series = 0, *ds = 0;

    series = (struct series_data *) malloc( sizeof *series );
    if( series )
    {
        series->next = 0;

        series->id = SVG_EMPTY_SERIES;
        series->cases = 0;
        series->circ_line_size = SVG_NO_VALUE;
        series->circ_radius = SVG_NO_VALUE;
        series->data_line_size = SVG_NO_VALUE;

        series->circ_fill_alpha = DEF_CIR_FILL_OP;
        series->circ_line_alpha = DEF_CIR_LIN_OP;
        series->data_fill_alpha = DEF_DAT_FILL_OP;
        series->data_line_alpha = DEF_DAT_LIN_OP;

        series->circ_fill_color = 0;
        series->circ_line_color = 0;
        series->data_fill_color = 0;
        series->data_line_color = 0;

        series->loc_xmin = 0;
        series->loc_xmax = 0;
        series->loc_ymin = 0;
        series->loc_ymax = 0;
        series->xdata = 0;
        series->ydata = 0;

        st = series->circ_fill_color = strdup( DEF_CIR_FILL_RGB );
        if( st ) st = series->circ_line_color = strdup( DEF_CIR_LIN_RGB );
        if( st ) st = series->data_fill_color = strdup( DEF_DAT_FILL_RGB );
        if( st ) st = series->data_line_color = strdup( DEF_DAT_LIN_RGB );
        if( st ) st = series->descr = strdup( EMPTY_STRING );

        if( !st )
        {
            if( series->circ_fill_color) free( series->circ_fill_color );
            if( series->circ_line_color) free( series->circ_line_color );
            if( series->data_fill_color) free( series->data_fill_color );
            if( series->data_line_color) free( series->data_line_color );
            if( series->descr) free( series->descr );
            free( series );
            series = 0;
	}
        else
        {
            if( !svg->series ) svg->series = series;
            else
            {
                for( ds = svg->series; ds->next; ds = ds->next ) ;
                ds->next = series;
	    }
	}
    }

    return( series );
}

/* --- */

struct svg_model *svg_make_chart()

{
    char *st = 0;
    struct svg_model *svg = 0;

    svg = (struct svg_model *) malloc( sizeof *svg );

    if( svg )
    {
        svg->series = 0;
        svg->total_cases = 0;

        svg->xmin = 0.0;
        svg->xmax = 0.0;
        svg->ymin = 0.0;
        svg->ymax = 0.0;

        svg->xmiles = 0;
        svg->ymiles = 0;

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
        svg->graph_area_width = SVG_NO_VALUE;
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
        svg->reserve_height = SVG_NO_VALUE;
        svg->reserve_width = SVG_NO_VALUE;
        svg->shift_width = SVG_NO_VALUE;
        svg->shift_height = SVG_NO_VALUE;
        svg->shift_bottom = SVG_NO_VALUE;
        svg->has_legend = DEF_HAS_LEGEND;
        svg->legend_scale = DEF_LEGEND_SCALE;
        svg->reserve_leg_width = 0;
        svg->xax_format_type = DTYPE_AUTO;
        svg->yax_format_type = DTYPE_AUTO;

        svg->leg_gap = DEF_LEG_GAP;
        svg->leg_cont_gap = DEF_LEG_CONT_GAP;
        svg->leg_col_pad = DEF_LEG_COL_PAD;
        svg->leg_row_pad = DEF_LEG_ROW_PAD;
        svg->leg_font_scale = DEF_LEG_FONT_SCALE;

        /* These legend parameters are computed from other values, just set to '0' */
        svg->leg_circ_col = 0;
        svg->leg_cont_left_col = 0;
        svg->leg_cont_top_row = 0;
        svg->leg_cont_width = 0;
        svg->leg_label_floor = 0;
        svg->leg_left_col = 0;
        svg->leg_line_len = 0;
        svg->leg_series_desc_col = 0;
        svg->leg_width = 0;
        svg->leg_line_height = 0;
        svg->leg_line_pad = SVG_NO_VALUE;

        svg->axis_alpha = DEF_AXIS_OP;
        svg->graph_alpha = DEF_GR_FILL_OP;
        svg->chart_alpha = DEF_BG_OP;
        svg->text_alpha = DEF_TEXT_OP;
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
        svg->text_color = 0;
        svg->x_gridline_color = 0;
        svg->y_gridline_color = 0;
        svg->xax_disp = 0;
        svg->yax_disp = 0;
        svg->screen_height = 0;
        svg->screen_width = 0;

        svg->matte_width = SVG_NO_VALUE;
        svg->matte_height = SVG_NO_VALUE;

        st = svg->chart_title = strdup( DEF_CHA_TITLE_TEXT );
        if( st ) st = svg->xax_title = strdup( DEF_XAXIS_TITLE );
        if( st ) st = svg->yax_title = strdup( DEF_YAXIS_TITLE );
        if( st ) st = svg->text_size = strdup( DEF_TEXT_SIZE );
        if( st ) st = svg->axis_color = strdup( DEF_AXIS_RGB );
        if( st ) st = svg->chart_color = strdup( DEF_BG_RGB );
        if( st ) st = svg->graph_color = strdup( DEF_GR_FILL_RGB );
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
        else
        {
            (void) add_data_series( svg );
            if( !svg->series )
            {
                free( svg );
                svg = 0;
	    }
	}

    }

    /* --- */

    return( svg );
}


/* --- */

void svg_free_model( struct svg_model *svg )

{
    struct svg_chart_milestone *ckpt, *walk;
    struct series_data *ds, *ds_next;

    if( svg )
    {
        for( ds = svg->series; ds; )
        {
            ds_next = ds->next;

            if( ds->xdata ) free( ds->xdata );
            if( ds->ydata ) free( ds->ydata );

            if( ds->circ_fill_color ) free( ds->circ_fill_color );
            if( ds->circ_line_color ) free( ds->circ_line_color );
            if( ds->data_fill_color ) free( ds->data_fill_color );
            if( ds->data_line_color ) free( ds->data_line_color );
            if( ds->descr ) free( ds->descr );

            ds->xdata = 0;
            ds->ydata = 0;

            ds->circ_fill_color = 0;
            ds->circ_line_color = 0;
            ds->data_fill_color = 0;
            ds->data_line_color = 0;
            ds->descr = 0;

            free( ds );
            ds = ds_next;
	}
        svg->series = 0;

        if( svg->chart_title ) free( svg->chart_title );
        if( svg->xax_title ) free( svg->xax_title );
        if( svg->yax_title ) free( svg->yax_title );
        if( svg->text_size ) free( svg->text_size );
        if( svg->axis_color ) free( svg->axis_color );
        if( svg->chart_color ) free( svg->chart_color );
        if( svg->graph_color ) free( svg->graph_color );
        if( svg->text_color ) free( svg->text_color );
        if( svg->x_gridline_color ) free( svg->x_gridline_color );
        if( svg->y_gridline_color ) free( svg->y_gridline_color );
        if( svg->xax_disp ) free( svg->xax_disp );
        if( svg->yax_disp ) free( svg->yax_disp );
        if( svg->screen_height ) free( svg->screen_height );
        if( svg->screen_width ) free( svg->screen_width );
        if( svg->svt_row_label ) free( svg->svt_row_label );
        if( svg->svt_row_line ) free( svg->svt_row_line );
        if( svg->svt_col_label ) free( svg->svt_col_label );
        if( svg->svt_col_line ) free( svg->svt_col_line );
        if( svg->svt_circ_elem ) free( svg->svt_circ_elem );
        if( svg->svt_path_start ) free( svg->svt_path_start );
        if( svg->svt_path_points ) free( svg->svt_path_points );
        if( svg->svt_chart ) free( svg->svt_chart );

        svg->chart_title = 0;
        svg->xax_title = 0;
        svg->yax_title = 0;
        svg->text_size = 0;
        svg->axis_color = 0;
        svg->chart_color = 0;
        svg->graph_color = 0;
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

        for( walk = svg->xmiles; walk; )
        {
            ckpt = walk;
            walk = walk->next;
            if( ckpt->label ) free( ckpt->label );
            if( ckpt->line_color ) free( ckpt->line_color );
            if( ckpt->text_color ) free( ckpt->text_color );
            if( ckpt->text_size ) free( ckpt->text_size );
            ckpt->label = 0;
            ckpt->line_color = 0;
            ckpt->text_color = 0;
            ckpt->text_size = 0;
            free( ckpt );
	}

        for( walk = svg->ymiles; walk; )
        {
            ckpt = walk;
            walk = walk->next;
            if( ckpt->label ) free( ckpt->label );
            if( ckpt->line_color ) free( ckpt->line_color );
            if( ckpt->text_color ) free( ckpt->text_color );
            if( ckpt->text_size ) free( ckpt->text_size );
            ckpt->label = 0;
            ckpt->line_color = 0;
            ckpt->text_color = 0;
            ckpt->text_size = 0;
            free( ckpt );
	}

        free( svg );
        svg = 0;
    }

    return;
}

/* --- */

int svg_finalize_model( struct svg_model *svg )

{
    int rc = RC_NORMAL, font_height, gap_size, cont_gap, col_pad, row_pad,
      symbol_wid = 0, leg_circ_radius = 0, leg_circ_lsize = 0, leg_data_lsize = 0,
      min_symbol_wid;
    struct svg_chart_milestone *ckpt = 0, *walk;
    struct series_data *ds = 0;

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
        if( svg->matte_width == SVG_NO_VALUE ) svg->matte_width = svg->chart_width * 0.5;
        if( svg->matte_height == SVG_NO_VALUE ) svg->matte_height = svg->chart_height * 0.5;

        for( ds = svg->series; ds; ds = ds->next )
        {
            if( !ds->descr ) ds->descr = strdup( EMPTY_STRING );

            if( ds->data_line_size == SVG_NO_VALUE ) ds->data_line_size = svg->chart_height * 0.00334;
            if( ds->circ_line_size == SVG_NO_VALUE ) ds->circ_line_size = svg->chart_height * 0.00334;
            if( ds->circ_radius == SVG_NO_VALUE ) ds->circ_radius = svg->chart_height * 0.02;

            if( ds->data_line_size > leg_data_lsize ) leg_data_lsize = ds->data_line_size;
            if( ds->circ_line_size > leg_circ_lsize ) leg_circ_lsize = ds->circ_line_size;
            if( ds->circ_radius > leg_circ_radius ) leg_circ_radius = ds->circ_radius;
            if( leg_circ_lsize + 2 * leg_circ_radius > symbol_wid ) symbol_wid = leg_circ_lsize + 2 * leg_circ_radius;

            if( symbol_wid > svg->leg_line_height ) svg->leg_line_height = symbol_wid;
            if( leg_data_lsize > svg->leg_line_height ) svg->leg_line_height = leg_data_lsize;
	}

        if( svg->x_gridline_size == SVG_NO_VALUE ) svg->x_gridline_size = svg->chart_height * 0.00334;
        if( svg->y_gridline_size == SVG_NO_VALUE ) svg->y_gridline_size = svg->chart_height * 0.00334;
        if( svg->xax_text_adj == SVG_NO_VALUE ) svg->xax_text_adj = svg->chart_height * 0.02;
        if( svg->yax_text_adj == SVG_NO_VALUE ) svg->yax_text_adj = svg->reserve_width * 0.15;
        if( svg->axis_size == SVG_NO_VALUE ) svg->axis_size = svg->chart_height * 0.00334;

        if( svg->graph_width == SVG_NO_VALUE ) svg->graph_width = svg->chart_width - (2 * svg->reserve_width) - svg->shift_width - svg->reserve_leg_width;
        if( svg->graph_area_width == SVG_NO_VALUE ) svg->graph_area_width = svg->graph_width;
        if( svg->graph_height == SVG_NO_VALUE ) svg->graph_height = svg->chart_height - (2 * svg->reserve_height) - svg->shift_height - svg->shift_bottom;

        if( svg->has_legend )
        {
            if( svg->legend_scale == SVG_NO_VALUE || svg->legend_scale < 1 ) svg->legend_scale = DEF_LEGEND_SCALE;
            svg->reserve_leg_width = (svg->graph_area_width * svg->legend_scale) / 100;
            svg->graph_width = svg->graph_area_width - svg->reserve_leg_width;
	}
        else
        {
            svg->reserve_leg_width = 0;
            svg->leg_circ_col = 0;
            svg->leg_cont_width = 0;
            svg->leg_cont_left_col = 0;
            svg->leg_cont_top_row = 0;
            svg->leg_font_scale = 0;
            svg->leg_label_floor = 0;
            svg->leg_left_col = 0;
            svg->leg_line_len = 0;
            svg->leg_width = 0;
            svg->leg_series_desc_col = 0;
            svg->leg_gap = 0;
	}

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

        if( svg->chart_height < svg->chart_width / 2 ) font_height = svg->chart_height / 20;
        else font_height = svg->chart_width / 40;

        if( svg->text_size ) if( !*svg->text_size )
        {
            free( svg->text_size);
            svg->text_size = 0;
	}

        if( !svg->text_size ) svg->text_size = string_from_int( &rc, font_height, 0);
        else if( !index( svg->text_size, '%' ) ) font_height = strtoul( svg->text_size, 0, BASE10 );

        /* --- */

        if( svg->has_legend )
        {
            if( svg->leg_font_scale == SVG_NO_VALUE || svg->leg_font_scale < 1 ) svg->leg_font_scale = DEF_LEG_FONT_SCALE;

            font_height -= (font_height * svg->leg_font_scale) / 100;
            if( font_height < SVG_MIN_FONT_HEIGHT ) font_height = SVG_MIN_FONT_HEIGHT;

            if( font_height > svg->leg_line_height) svg->leg_line_height = font_height;

            if( svg->leg_gap == SVG_NO_VALUE ) svg->leg_gap = DEF_LEG_GAP;
            if( svg->leg_cont_gap == SVG_NO_VALUE ) svg->leg_cont_gap = DEF_LEG_CONT_GAP;
            if( svg->leg_col_pad == SVG_NO_VALUE ) svg->leg_col_pad = DEF_LEG_COL_PAD;
            if( svg->leg_row_pad == SVG_NO_VALUE ) svg->leg_row_pad = DEF_LEG_ROW_PAD;
            if( svg->leg_line_pad == SVG_NO_VALUE ) svg->leg_line_pad = DEF_LEG_LINE_PAD;

            svg->leg_width = svg->reserve_leg_width;

            row_pad = (svg->graph_height * svg->leg_row_pad) / 100;
            if( row_pad < 1 ) row_pad = 0;

            gap_size = (svg->leg_width * svg->leg_gap) / 100;
            if( gap_size < SVG_MIN_LEGEND_GAP ) gap_size = SVG_MIN_LEGEND_GAP;

            svg->leg_cont_width = svg->leg_width - gap_size;

            symbol_wid += 2 * leg_circ_radius;
            min_symbol_wid = (svg->leg_cont_width * SVG_MIN_LEGEND_SYMBOL_WIDTH) / 100;
            if( symbol_wid < min_symbol_wid ) symbol_wid = min_symbol_wid;

            col_pad = (svg->leg_cont_width * svg->leg_col_pad) / 100;
            if( col_pad < 1 ) col_pad = 0;

            cont_gap = (svg->leg_cont_width * svg->leg_cont_gap) / 100;
            if( cont_gap < 1 ) cont_gap = 0;

            svg->leg_left_col = svg->graph_left_col + svg->graph_width + gap_size;
            svg->leg_cont_left_col = svg->leg_left_col + col_pad;
            svg->leg_circ_col = svg->leg_cont_left_col + symbol_wid / 2;
            svg->leg_cont_top_row = svg->graph_top_row + row_pad + ( font_height / 2 );
            svg->leg_label_floor = svg->leg_cont_top_row + font_height;
            svg->leg_line_len = symbol_wid;
            svg->leg_series_desc_col = svg->leg_cont_left_col + symbol_wid + cont_gap;
	}

        for( ckpt = svg->xmiles; ckpt; ckpt = ckpt->next)
        {
            if( ckpt->offset < svg->xmin) svg->xmin = ckpt->offset;
            if( ckpt->offset > svg->xmax) svg->xmax = ckpt->offset;
	}

        for( ckpt = svg->ymiles; ckpt; ckpt = ckpt->next)
        {
            if( ckpt->offset < svg->ymin) svg->ymin = ckpt->offset;
            if( ckpt->offset > svg->ymax) svg->ymax = ckpt->offset;
	}

        walk = svg->xmiles;
        if( !walk) walk = svg->ymiles;

        for( ; walk && rc == RC_NORMAL; )
        {
            for( ckpt = walk; ckpt && rc == RC_NORMAL; ckpt = ckpt->next)
            {
                if( ckpt->width == SVG_NO_VALUE ) ckpt->width = svg->chart_height * 0.015555;
                if( ckpt->extend == SVG_NO_VALUE ) ckpt->extend = DEF_MSTONE_EXTEND;
                if( ckpt->text_alpha == SVG_NO_VALUE ) ckpt->text_alpha = DEF_MSTONE_TEXT_OP;
                if( ckpt->line_alpha == SVG_NO_VALUE ) ckpt->line_alpha = DEF_MSTONE_LINE_OP;

                if( !ckpt->label ) ckpt->label = strdup( DEF_MSTONE_LABEL );

                if( ckpt->text_size) if( !*ckpt->text_size )
                {
                    free( ckpt->text_size );
                    ckpt->text_size = 0;
		}
                if( !ckpt->text_size ) ckpt->text_size = string_from_int( &rc, svg->chart_height / 20, 0);

                if( ckpt->text_color) if( !*ckpt->text_color )
                {
                    free( ckpt->text_color );
                    ckpt->text_color = 0;
		}
                if( !ckpt->text_color ) ckpt->text_color = strdup( DEF_MSTONE_TEXT_RGB );

                if( ckpt->line_color) if( !*ckpt->line_color )
                {
                    free( ckpt->line_color );
                    ckpt->line_color = 0;
		}
                if( !ckpt->line_color ) ckpt->line_color = strdup( DEF_MSTONE_LINE_RGB );

                if( !ckpt->label || !ckpt->text_size || !ckpt->text_color 
                  || !ckpt->line_color ) rc = ERR_MALLOC_FAILED;
	    }

           if( walk != svg->ymiles ) walk = svg->ymiles;
	}
    }

    return( rc );
}

/* --- */

struct series_data *svg_add_double_data( int *rc, struct svg_model *svg, int cases, double *xval, double *yval)

{
    int off, maxid;
    double *xdata = 0, *ydata = 0, xmin, ymin, xmax, ymax, xd, yd;
    struct series_data *slot = 0, *walk;

    if( !svg || cases <= 0 ) *rc = ERR_UNSUPPORTED;
    else
    {
        slot = get_empty_data_series( svg );
        if( !slot ) *rc = ERR_MALLOC_FAILED;
        else
        {
            maxid = 0;
            for( walk = svg->series; walk; walk = walk->next )
              if( walk->id > maxid ) maxid = walk->id;
            slot->id = maxid + 1;
	}
    }
        
    if( *rc == RC_NORMAL )
    {
        xdata = (double *) malloc( cases * (sizeof *xdata) );
        ydata = (double *) malloc( cases * (sizeof *ydata) );

        if( !xdata || !ydata )
        {
            *rc = ERR_MALLOC_FAILED;
            if( xdata ) free( xdata );
            if( ydata ) free( ydata );
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

            slot->cases = cases;
            slot->loc_xmin = xmin;
            slot->loc_xmax = xmax;
            slot->loc_ymin = ymin;
            slot->loc_ymax = ymax;
            slot->xdata = xdata;
            slot->ydata = ydata;

            svg->total_cases += cases;
            if( slot->id == 1 || xmin < svg->xmin ) svg->xmin = xmin;
            if( slot->id == 1 || xmax > svg->xmax ) svg->xmax = xmax;
            if( slot->id == 1 || ymin < svg->ymin ) svg->ymin = ymin;
            if( slot->id == 1 || ymax > svg->ymax ) svg->ymax = ymax;
	}
    }

    return( slot );
}

/* --- */

struct series_data *svg_add_float_data( int *rc, struct svg_model *svg, int cases, float *xval, float *yval)

{
    int off, maxid;
    double *xdata = 0, *ydata = 0, xmin, ymin, xmax, ymax, xd, yd;
    struct series_data *slot = 0, *walk;

    if( !svg || cases <= 0 ) *rc = ERR_UNSUPPORTED;
    else
    {
        slot = get_empty_data_series( svg );
        if( !slot ) *rc = ERR_MALLOC_FAILED;
        else
        {
            maxid = 0;
            for( walk = svg->series; walk; walk = walk->next )
              if( walk->id > maxid ) maxid = walk->id;
            slot->id = maxid + 1;
	}
    }
        
    if( *rc == RC_NORMAL )
    {
        xdata = (double *) malloc( cases * (sizeof *xdata) );
        ydata = (double *) malloc( cases * (sizeof *ydata) );

        if( !xdata || !ydata )
        {
            *rc = ERR_MALLOC_FAILED;
            if( xdata ) free( xdata );
            if( ydata ) free( ydata );
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

            slot->cases = cases;
            slot->loc_xmin = xmin;
            slot->loc_xmax = xmax;
            slot->loc_ymin = ymin;
            slot->loc_ymax = ymax;
            slot->xdata = xdata;
            slot->ydata = ydata;

            svg->total_cases += cases;
            if( slot->id == 1 || xmin < svg->xmin ) svg->xmin = xmin;
            if( slot->id == 1 || xmax > svg->xmax ) svg->xmax = xmax;
            if( slot->id == 1 || ymin < svg->ymin ) svg->ymin = ymin;
            if( slot->id == 1 || ymax > svg->ymax ) svg->ymax = ymax;
	}
    }

    return( slot );
}

/* --- */

struct series_data *svg_add_int_data( int *rc, struct svg_model *svg, int cases, int *xval, int *yval)

{
    int off, maxid;
    double *xdata = 0, *ydata = 0, xmin, ymin, xmax, ymax, xd, yd;
    struct series_data *slot = 0, *walk;

    if( !svg || cases <= 0 ) *rc = ERR_UNSUPPORTED;
    else
    {
        slot = get_empty_data_series( svg );
        if( !slot ) *rc = ERR_MALLOC_FAILED;
        else
        {
            maxid = 0;
            for( walk = svg->series; walk; walk = walk->next )
              if( walk->id > maxid ) maxid = walk->id;
            slot->id = maxid + 1;
	}
    }
        
    if( *rc == RC_NORMAL )
    {
        xdata = (double *) malloc( cases * (sizeof *xdata) );
        ydata = (double *) malloc( cases * (sizeof *ydata) );

        if( !xdata || !ydata )
        {
            *rc = ERR_MALLOC_FAILED;
            if( xdata ) free( xdata );
            if( ydata ) free( ydata );
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

            slot->cases = cases;
            slot->loc_xmin = xmin;
            slot->loc_xmax = xmax;
            slot->loc_ymin = ymin;
            slot->loc_ymax = ymax;
            slot->xdata = xdata;
            slot->ydata = ydata;

            svg->total_cases += cases;
            if( slot->id == 1 || xmin < svg->xmin ) svg->xmin = xmin;
            if( slot->id == 1 || xmax > svg->xmax ) svg->xmax = xmax;
            if( slot->id == 1 || ymin < svg->ymin ) svg->ymin = ymin;
            if( slot->id == 1 || ymax > svg->ymax ) svg->ymax = ymax;
	}
    }

    return( slot );
}

/* --- */

char *svg_make_yax_labels( int *rc, struct svg_model *svg )

{
    int point;
    char *svg_part = 0, *template, *one_label = 0, *accumulate = 0;
    float label, label_incr = 0.0;
    struct sub_list *subs = 0, *label_sub = 0, *first_ypos = 0, *walk = 0;

    if( *rc == RC_NORMAL && !svg ) *rc = ERR_UNSUPPORTED;

    if( *rc == RC_NORMAL )
    {
        label_incr = (svg->ymax - svg->ymin) / (float) svg->yax_num_grids;
        label = svg->ymin;

        template = svg->svt_row_label;

	subs = (struct sub_list *) malloc( sizeof *subs );
        first_ypos = (struct sub_list *) malloc( sizeof *first_ypos );

        if( !subs || !first_ypos ) *rc = ERR_MALLOC_FAILED;
        else 
        {
            label_sub = subs;
            label_sub->from = S_LABEL;
            label_sub->to = 0;
            label_sub->next = 0;

            first_ypos->from = S_YAXIS_HI;
            first_ypos->to = strdup( YAX_LABEL_FIRST_YPOS );
            first_ypos->next = label_sub;
	}
    }

    if( *rc == RC_NORMAL )
    {
#ifdef FORMAT_ONLY_AS_FLOAT
        label_sub->to = string_from_float( rc, label, svg->yax_disp );
#else
        label_sub->to = string_from_typed_format( rc, (void *) &label, svg->yax_format_type, svg->yax_disp );
#endif
        if( !label_sub->to ) *rc = ERR_UNSUPPORTED;
        else svg_part = gsub_string( rc, template, first_ypos );
    }

    for( point = 1; *rc == RC_NORMAL && point <= svg->yax_num_grids; point++)
    {
        label += label_incr;

        if( label_sub->to ) free( label_sub->to );
#ifdef FORMAT_ONLY_AS_FLOAT
        label_sub->to = string_from_float( rc, label, svg->yax_disp );
#else
        label_sub->to = string_from_typed_format( rc, (void *) &label, svg->yax_format_type, svg->yax_disp );
#endif
        if( !label_sub->to ) *rc = ERR_UNSUPPORTED;
        else
        {
            one_label = gsub_string( rc, template, subs );
            accumulate = combine_strings( rc, svg_part, one_label );

            if( one_label ) free( one_label );
            one_label = 0;

            if( svg_part ) free( svg_part );
            svg_part = accumulate;
            accumulate = 0;
	}
    }
    
    for( walk = first_ypos; walk; )
    {
        subs = walk->next;
        if( walk->to ) free( walk->to );
        free( walk );
        walk = subs;
    }

    /* --- */

    if( *rc != RC_NORMAL && svg_part )
    {
        free( svg_part );
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
    
    for( point = 0; *rc == RC_NORMAL && point <= svg->xax_num_grids; point++)
    {
        xpos = svg->graph_left_col + (int) xpos_adj;

        if( label_sub->to ) free( label_sub->to );
        if( xpos_sub->to ) free( xpos_sub->to );

#ifdef FORMAT_ONLY_AS_FLOAT
        label_sub->to = string_from_float( rc, label, svg->xax_disp );
#else
        label_sub->to = string_from_typed_format( rc, (void *) &label, svg->xax_format_type, svg->xax_disp );
#endif
        xpos_sub->to = string_from_int( rc, xpos, 0 );

        if( !label_sub->to || !xpos_sub->to ) *rc = ERR_UNSUPPORTED;
        else
        {
            one_label = gsub_string( rc, template, subs );
            accumulate = combine_strings( rc, svg_part, one_label );

            if( one_label ) free( one_label );
            one_label = 0;

            if( svg_part ) free( svg_part );
            svg_part = accumulate;
            accumulate = 0;
	}

        label += label_incr;
        xpos_adj += xpos_incr;
    }
    
    for( walk = subs; walk; )
    {
        subs = walk->next;
        if( walk->to ) free( walk->to );
        free( walk );
        walk = subs;
    }

    /* --- */

    if( *rc != RC_NORMAL && svg_part )
    {
        free( svg_part );
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
        free( svg_part );
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
        free( svg_part );
        svg_part = 0;
    }

    return( svg_part );
}

/* --- */

int update_series_point_subs( struct sub_list **subs, struct svg_model *svg, struct series_data *ds )

{
    int *rc, stat = RC_NORMAL;
    char *specs = 0, *replace = 0, *intermed = 0;
    struct sub_list *rule = 0, *pending = 0, *walk = 0;

    rc = &stat;

    intermed = svg_make_data_points( rc, svg, ds );

    if( *rc == RC_NORMAL )
    {
        if( !*subs )
        {
            ADD_SUB_PAIR_RULE( S_CIR_FILL_RGB, strdup( ds->circ_fill_color ) )
            *subs = rule;
            ADD_SUB_PAIR_RULE( S_CIR_LIN_RGB, strdup( ds->circ_line_color ) )
            ADD_SUB_PAIR_RULE( S_CIR_FILL_OP, string_from_float( rc, ds->circ_fill_alpha, ALPHA_DISP_FORMAT ) )
            ADD_SUB_PAIR_RULE( S_CIR_LIN_OP, string_from_float( rc, ds->circ_line_alpha, ALPHA_DISP_FORMAT ) )
            ADD_SUB_PAIR_RULE( S_CIR_LIN_SIZE, string_from_int( rc, ds->circ_line_size, 0 ) )
            ADD_SUB_PAIR_RULE( S_CIR_RAD, string_from_int( rc, ds->circ_radius, 0 ) )

            if( *rc == RC_NORMAL )
            {
                specs = gsub_string( rc, intermed, *subs );
                if( intermed ) free( intermed );
                ADD_SUB_PAIR_RULE( S_ST_DATA_POINT, specs )
	    }
	}
        else for( walk = *subs; *rc == RC_NORMAL && walk; walk = walk->next )
        {
            replace = 0;
            if( !strcmp(walk->from, S_CIR_FILL_RGB) ) replace = strdup( ds->circ_fill_color );
            else if( !strcmp(walk->from, S_CIR_LIN_RGB) ) replace = strdup( ds->circ_line_color );
            else if( !strcmp(walk->from, S_CIR_FILL_OP) ) replace = string_from_float( rc, ds->circ_fill_alpha, ALPHA_DISP_FORMAT );
            else if( !strcmp(walk->from, S_CIR_LIN_OP) ) replace = string_from_float( rc, ds->circ_line_alpha, ALPHA_DISP_FORMAT );
            else if( !strcmp(walk->from, S_CIR_LIN_SIZE) ) replace = string_from_int( rc, ds->circ_line_size, 0 );
            else if( !strcmp(walk->from, S_CIR_RAD) ) replace = string_from_int( rc, ds->circ_radius, 0 );
            else if( !strcmp(walk->from, S_ST_DATA_POINT) )
            {
                replace = gsub_string( rc, intermed, *subs );
                if( intermed ) free( intermed );
	    }

            if( replace )
            {
                free( walk->to );
                walk->to = replace;
	    }
	}
    }

    return( stat );
}

/* --- */

int update_series_line_subs( struct sub_list **subs, struct svg_model *svg, struct series_data *ds )

{
    int *rc, stat = RC_NORMAL;
    char *path_specs = 0, *line_specs = 0, *replace = 0;
    struct sub_list *rule = 0, *pending = 0, *walk = 0;

    rc = &stat;

    path_specs = svg_make_path_start( rc, svg, ds );
    if( *rc == RC_NORMAL ) line_specs = svg_make_data_lines( rc, svg, ds );

    if( *rc == RC_NORMAL )
    {
        if( !*subs )
        {
            ADD_SUB_PAIR_RULE( S_DAT_FILL_RGB, strdup( ds->data_fill_color ) )
            *subs = rule;
            ADD_SUB_PAIR_RULE( S_DAT_LIN_RGB, strdup( ds->data_line_color ) )
            ADD_SUB_PAIR_RULE( S_DAT_FILL_OP, string_from_float( rc, ds->data_fill_alpha, ALPHA_DISP_FORMAT ) )
            ADD_SUB_PAIR_RULE( S_DAT_LIN_OP, string_from_float( rc, ds->data_line_alpha, ALPHA_DISP_FORMAT ) )
            ADD_SUB_PAIR_RULE( S_DAT_LIN_SIZE, string_from_int( rc, ds->data_line_size, 0 ) )
            ADD_SUB_PAIR_RULE( S_ST_START_PATH, path_specs )
            ADD_SUB_PAIR_RULE( S_ST_DATA_LINE, line_specs )
	}
        else for( walk = *subs; *rc == RC_NORMAL && walk; walk = walk->next )
        {
            replace = 0;
            if( !strcmp(walk->from, S_DAT_FILL_RGB) ) replace = strdup( ds->data_fill_color );
            else if( !strcmp(walk->from, S_DAT_LIN_RGB) ) replace = strdup( ds->data_line_color );
            else if( !strcmp(walk->from, S_DAT_FILL_OP) ) replace = string_from_float( rc, ds->data_fill_alpha, ALPHA_DISP_FORMAT );
            else if( !strcmp(walk->from, S_DAT_LIN_OP) ) replace = string_from_float( rc, ds->data_line_alpha, ALPHA_DISP_FORMAT );
            else if( !strcmp(walk->from, S_DAT_LIN_SIZE) ) replace = string_from_int( rc, ds->data_line_size, 0 );
            else if( !strcmp(walk->from, S_ST_START_PATH) ) replace = path_specs;
            else if( !strcmp(walk->from, S_ST_DATA_LINE) ) replace = line_specs;

            if( replace )
            {
                free( walk->to );
                walk->to = replace;
	    }
	}
    }

    return( stat );
}

/* --- */

char *svg_make_series_points( int *rc, struct svg_model *svg )

{
    char *all_points = 0, *point_seg = 0, *template, *agg = 0;
    struct series_data *ds;
    struct sub_list *subs = 0, *walk = 0;

    template = strdup( SVG_SERIES_POINTS );
    if( !template ) *rc = ERR_MALLOC_FAILED;

    for( ds = svg->series; *rc == RC_NORMAL && ds; ds = ds->next )
    {
        *rc = update_series_point_subs( &subs, svg, ds );
        if( *rc == RC_NORMAL ) point_seg = gsub_string( rc, template, subs );

        agg = combine_strings( rc, all_points, point_seg );

        if( *rc == RC_NORMAL )
        {
            if( all_points ) free( all_points );
            if( point_seg ) free( point_seg );
            all_points = agg;
	}
    }

    if( template ) free( template );

    for( walk = subs; walk; )
    {
        subs = walk->next;
        free( walk->to );
        free( walk );
        walk = subs;
    }

    return( all_points );
}

/* --- */

char *svg_make_data_points( int *rc, struct svg_model *svg, struct series_data *ds )

{
    int xpos, ypos, xbase, ybase, xscale, yscale, pt;
    float xpc, ypc;
    double *xdata, *ydata, xval, yval;
    char *spots = 0, *template = 0, *acc = 0, *one_line = 0;
    struct sub_list *xpos_sub, *ypos_sub, *subs = 0, *walk = 0;

    ENTER( "svg_make_data_points" )

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

        xdata = ds->xdata;
        ydata = ds->ydata;

        for( pt = 0; pt < ds->cases && *rc == RC_NORMAL; pt++ )
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

    LEAVE( "svg_make_data_points" )
    return( spots );
}

/* --- */

char *svg_make_series_lines( int *rc, struct svg_model *svg )

{
    char *all_lines = 0, *line_seg = 0, *agg = 0, *template = 0;
    struct series_data *ds;
    struct sub_list *subs = 0, *walk = 0;

    template = strdup( SVG_SERIES_LINE );
    if( !template ) *rc = ERR_MALLOC_FAILED;

    for( ds = svg->series; *rc == RC_NORMAL && ds; ds = ds->next )
    {
        *rc = update_series_line_subs( &subs, svg, ds );
        if( *rc == RC_NORMAL) line_seg = gsub_string( rc, template, subs );

        agg = combine_strings( rc, all_lines, line_seg );

        if( *rc == RC_NORMAL )
        {
            if( all_lines ) free( all_lines );
            if( line_seg ) free( line_seg );
            all_lines = agg;
	}
    }

    if( template ) free( template );

    for( walk = subs; walk; )
    {
        subs = walk->next;
        free( walk->to );
        free( walk );
        walk = subs;
    }

    return( all_lines );   
}

/* --- */

char *svg_make_path_start( int *rc, struct svg_model *svg, struct series_data *ds )

{
    int xpos, ypos, xscale, yscale;
    float xpc, ypc;
    char *pstart = 0, *template = 0;
    struct sub_list *xpos_sub, *ypos_sub, *subs = 0, *walk = 0;

    if( *rc == RC_NORMAL )
    {
        template = svg->svt_path_start;

        xscale = svg->xax_border - svg->graph_left_col;
        xpc = (*ds->xdata - svg->xmin) / (svg->xmax - svg->xmin);
        xpos = svg->graph_left_col + (xscale * xpc);

        yscale = svg->yax_border - svg->graph_top_row;
        ypc = (*ds->ydata - svg->ymin) / (svg->ymax - svg->ymin); 
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

char *svg_make_data_lines( int *rc, struct svg_model *svg, struct series_data *ds )

{
    int xpos, ypos, xbase, ybase, xscale, yscale, pt;
    float xpc, ypc;
    double *xdata, *ydata, xval, yval;
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

        xdata = ds->xdata;
        ydata = ds->ydata;

        for( pt = 0; pt < ds->cases && *rc == RC_NORMAL; pt++ )
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

int update_leg_series_subs( struct sub_list **subs, struct svg_model *svg, struct series_data *ds )

{
    int *rc, stat = RC_NORMAL, pnum, n_parms;
    char *sp, *val = 0, *id_string;
    char *series_parms[] = { S_CIR_FILL_RGB, S_CIR_LIN_RGB, S_CIR_FILL_OP, S_CIR_LIN_OP,
      S_CIR_LIN_SIZE, S_CIR_RAD, S_DAT_FILL_RGB, S_DAT_LIN_RGB, S_DAT_FILL_OP, S_DAT_LIN_OP,
      S_DAT_LIN_SIZE, S_LEG_SER_DESC_TXT };
    struct sub_list *rule = 0, *pending = 0, *walk = 0, *match = 0;

    rc = &stat;

    if( *rc == RC_NORMAL )
    {
        n_parms = (sizeof series_parms) / (sizeof series_parms[0]);

	for( walk = *subs; walk; walk = walk->next ) rule = walk;

        for( pnum = 0; pnum < n_parms; pnum++ )
        {
            sp = series_parms[pnum];

	    if( !strcmp(sp, S_CIR_FILL_RGB ) ) val = strdup( ds->circ_fill_color );
            else if( !strcmp(sp, S_CIR_LIN_RGB ) ) val = strdup( ds->circ_line_color );
            else if( !strcmp(sp, S_CIR_FILL_OP ) ) val = string_from_float( rc, ds->circ_fill_alpha, ALPHA_DISP_FORMAT );
            else if( !strcmp(sp, S_CIR_LIN_OP ) ) val = string_from_float( rc, ds->circ_line_alpha, ALPHA_DISP_FORMAT );
            else if( !strcmp(sp, S_CIR_LIN_SIZE ) ) val = string_from_int( rc, ds->circ_line_size, 0 );
            else if( !strcmp(sp, S_CIR_RAD ) ) val = string_from_int( rc, ds->circ_radius, 0 );
            else if( !strcmp(sp, S_DAT_FILL_RGB ) ) val = strdup( ds->data_fill_color );
            else if( !strcmp(sp, S_DAT_LIN_RGB ) ) val = strdup( ds->data_line_color );
            else if( !strcmp(sp, S_DAT_FILL_OP ) ) val = string_from_float( rc, ds->data_fill_alpha, ALPHA_DISP_FORMAT );
            else if( !strcmp(sp, S_DAT_LIN_OP ) ) val = string_from_float( rc, ds->data_line_alpha, ALPHA_DISP_FORMAT );
            else if( !strcmp(sp, S_DAT_LIN_SIZE ) ) val = string_from_int( rc, ds->data_line_size, 0 );
            else if( !strcmp(sp, S_LEG_SER_DESC_TXT ) )
            {
                if( ds->descr ) if( *ds->descr ) val = strdup( ds->descr );
                if( !val )
                {
                    id_string = string_from_int( rc, ds->id, 0 );
                    if( id_string )
                    {
                        val = combine_strings( rc, "Series #", id_string );
                        free( id_string );
		    }
		}
                if( !val ) val = strdup( NO_SERIES_DESC );
	    }
            else *rc = ERR_UNSUPPORTED;

            if( *rc == RC_NORMAL && !val ) *rc = ERR_MALLOC_FAILED;

            if( *rc == RC_NORMAL )
            {
                match = 0;
                for( walk = *subs; !match && walk; walk = walk->next)
                {
                    if( !strcmp( sp, walk->from ) ) match = walk;
                }

                if( match )
                {
                    if( match->to ) free( match->to );
                    match->to = val;
                    val = 0;
                }
                else
                {
                    ADD_SUB_PAIR_RULE( sp, val )
                    if( !*subs ) *subs = rule;
                    val = 0;
		}
	    }

            if( val )
            {
                free( val );
                val = 0;
	    }
	}
    }

    return( *rc );
}

/* --- */

char *svg_make_legend( int *rc, struct svg_model *svg )

{
    int ypos, line_space;
    char *legend = 0, *base = 0, *seg, *agg, *all_points = 0, *all_lines = 0, *all_desc = 0,
      *circ_template, *line_template, *desc_template;
    struct series_data *ds = 0;
    struct sub_list *subs = 0, *rule = 0, *pending = 0, *ypos_sub, *walk
;

    if( *rc == RC_NORMAL )
    {
        if( svg->has_legend )
        {
            ADD_SUB_PAIR_RULE( S_LEG_CIR_COL, string_from_int( rc, svg->leg_circ_col, 0 ) )
            subs = rule;
            ADD_SUB_PAIR_RULE( S_LEG_CONT_WID, string_from_int( rc, svg->leg_cont_width, 0 ) )
            ADD_SUB_PAIR_RULE( S_LEG_CONT_LEFT_COL, string_from_int( rc, svg->leg_cont_left_col, 0 ) )
            ADD_SUB_PAIR_RULE( S_LEG_CONT_TOP_ROW, string_from_int( rc, svg->leg_cont_top_row, 0 ) )
            ADD_SUB_PAIR_RULE( S_LEG_FONT_SCALE, string_from_int( rc, svg->leg_font_scale, 0 ) )
            ADD_SUB_PAIR_RULE( S_LEG_LABEL_FLOOR, string_from_int( rc, svg->leg_label_floor, 0 ) )
            ADD_SUB_PAIR_RULE( S_LEG_LEFT_COL, string_from_int( rc, svg->leg_left_col, 0 ) )
            ADD_SUB_PAIR_RULE( S_LEG_LINE_LEN, string_from_int( rc, svg->leg_line_len, 0 ) )
            ADD_SUB_PAIR_RULE( S_LEG_WIDTH, string_from_int( rc, svg->leg_width, 0 ) )
            ADD_SUB_PAIR_RULE( S_LEG_SER_DESC_COL, string_from_int( rc, svg->leg_series_desc_col, 0 ) )

            ADD_SUB_PAIR_RULE( S_YPOS, string_from_int( rc, 0, 0) )
            ypos_sub = rule;

            if( *rc == RC_NORMAL )
            {
                base = gsub_string( rc, LEGEND_TEMPLATE, subs );

                circ_template = LEGEND_CIRCLE_TEMPLATE;
                line_template = LEGEND_LINE_TEMPLATE;
                desc_template = LEGEND_SERIES_TEMPLATE;

                /* --- */

                line_space = svg->leg_line_height + (svg->leg_line_height * svg->leg_line_pad) / 100;
/*                ypos = svg->leg_cont_top_row; */
                ypos = svg->leg_label_floor - (svg->leg_line_height * svg->leg_line_pad) / 100;

                for( ds = svg->series; *rc == RC_NORMAL && ds; ds = ds->next )
                {
                    *rc = update_leg_series_subs( &subs, svg, ds );

                    ypos += line_space;
                    if( ypos_sub->to ) free( ypos_sub->to );
                    ypos_sub->to = string_from_int( rc, ypos, 0 );

                    if( *rc == RC_NORMAL) seg = gsub_string( rc, circ_template, subs );
                    agg = combine_strings( rc, all_points, seg );
                    if( all_points ) free( all_points );
                    all_points = agg;
                    if( seg ) free( seg );
		    seg = 0;

                    if( *rc == RC_NORMAL) seg = gsub_string( rc, line_template, subs );
                    agg = combine_strings( rc, all_lines, seg );
                    if( all_lines ) free( all_lines );
                    all_lines = agg;
                    if( seg ) free( seg );
		    seg = 0;

                    if( *rc == RC_NORMAL) seg = gsub_string( rc, desc_template, subs );
                    agg = combine_strings( rc, all_desc, seg );
                    if( all_desc ) free( all_desc );
                    all_desc = agg;
                    if( seg ) free( seg );
		    seg = 0;
		}

                /* --- */

                if( !all_points )
                {
                    all_points = strdup( EMPTY_LEG_POINTS );
                    if( !all_points ) *rc = ERR_MALLOC_FAILED;
		}

                if( !all_lines )
                {
                    all_lines = strdup( EMPTY_LEG_LINES );
                    if( !all_lines ) *rc = ERR_MALLOC_FAILED;
		}

                if( !all_desc )
                {
                    all_desc = strdup( EMPTY_LEG_DESC );
                    if( !all_desc ) *rc = ERR_MALLOC_FAILED;
		}

                /* --- */

                if( *rc == RC_NORMAL )
                {
                    for( walk = subs; walk; walk = walk->next ) rule = walk;

                    ADD_SUB_PAIR_RULE( S_ST_LEG_CIR_ENTRIES, all_points )
                    ADD_SUB_PAIR_RULE( S_ST_LEG_LINE_ENTRIES, all_lines )
                    ADD_SUB_PAIR_RULE( S_ST_LEG_SERIES_ENTRIES, all_desc )
                    all_points = all_lines = all_desc = 0;
		}

                /* --- */

                /* Fold the circles, lines and descriptions into the legend base template */

                if( *rc == RC_NORMAL ) legend = gsub_string( rc, base, subs );

                if( all_points )
                {
                    free( all_points );
                    all_points = 0;
		}

                if( all_lines )
                {
                    free( all_lines );
                    all_lines = 0;
		}

                if( all_desc )
                {
                    free( all_desc );
                    all_desc = 0;
		}
	    }

            if( base )
            {
                free( base );
                base = 0;
	    }
	}

	for( rule = subs; rule; )
        {
            if( rule->to ) free( rule->to );
            subs = rule->next;
            free( rule );
            rule = subs;
        }
    
        subs = 0;

        /* --- */

        if( !legend )
        {
            legend = strdup( DEF_LEGEND_TABLE );
            if( !legend ) *rc = ERR_MALLOC_FAILED;
	}
    }

    return( legend );
}

/* --- */

char *svg_render( int *rc, struct svg_model *svg )

{
    char *chart = 0, *template = 0, *xax_label = 0, *yax_label = 0,
      *xax_grid = 0, *yax_grid = 0, *data_points = 0, *path_start = 0,
      *data_lines = 0, *xax_milestones = 0, *yax_milestones = 0,
      *series_points = 0, *series_lines = 0, *legend_table = 0;
    struct sub_list *subs = 0, *walk = 0, *rule = 0, *pending = 0;

    ENTER( "svg_render" )

    if( *rc == RC_NORMAL ) *rc = svg_finalize_model( svg );

    INSUB( "svg_render", "before-make-sublist" )

    if( *rc == RC_NORMAL ) subs = svg_make_sublist( rc, svg );

    INSUB( "svg_render", "before-make-xax-labels" )

    if( *rc == RC_NORMAL )
    {
        template = svg_make_xax_labels( rc, svg );
        if( *rc == RC_NORMAL) xax_label = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    INSUB( "svg_render", "before-make-yax-labels" )

    if( *rc == RC_NORMAL )
    {
        template = svg_make_yax_labels( rc, svg );
        if( *rc == RC_NORMAL) yax_label = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    INSUB( "svg_render", "before-make-xax-grid" )

    if( *rc == RC_NORMAL )
    {
        template = svg_make_xax_grid( rc, svg );
        if( *rc == RC_NORMAL) xax_grid = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    INSUB( "svg_render", "before-make-yax-grid" )

    if( *rc == RC_NORMAL )
    {
        template = svg_make_yax_grid( rc, svg );
        if( *rc == RC_NORMAL) yax_grid = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    INSUB( "svg_render", "before-make-data-points" )

    if( *rc == RC_NORMAL )
    {
        template = svg_make_data_points( rc, svg, svg->series );
        if( *rc == RC_NORMAL) data_points = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    INSUB( "svg_render", "before-make-path" )

    if( *rc == RC_NORMAL )
    {
        template = svg_make_path_start( rc, svg, svg->series );
        if( *rc == RC_NORMAL) path_start = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    INSUB( "svg_render", "before-make-data-lines" )

    if( *rc == RC_NORMAL )
    {
        template = svg_make_data_lines( rc, svg, svg->series );
        if( *rc == RC_NORMAL) data_lines = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    INSUB( "svg_render", "before-make-series-points" )

    if( *rc == RC_NORMAL )
    {
        template = svg_make_series_points( rc, svg );
        if( *rc == RC_NORMAL) series_points = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    INSUB( "svg_render", "before-make-series-lines" )

    if( *rc == RC_NORMAL )
    {
        template = svg_make_series_lines( rc, svg );
        if( *rc == RC_NORMAL) series_lines = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    INSUB( "svg_render", "before-make-xax-mstones" )

    if( *rc == RC_NORMAL )
    {
        xax_milestones = svg_make_xax_mstones( rc, svg );
    }

    INSUB( "svg_render", "before-make-yax-mstones" )

    if( *rc == RC_NORMAL )
    {
        yax_milestones = svg_make_yax_mstones( rc, svg );
    }

    INSUB( "svg_render", "before-legend" )

    if( *rc == RC_NORMAL )
    {
        template = svg_make_legend( rc, svg );
        if( *rc == RC_NORMAL) legend_table = gsub_string( rc, template, subs );
        if( template ) free( template );
    }

    INSUB( "svg_render", "before-add-subs" )

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
        ADD_SUB_PAIR_RULE( S_ST_SERIES_POINTS, series_points )
        ADD_SUB_PAIR_RULE( S_ST_SERIES_LINES, series_lines )
        ADD_SUB_PAIR_RULE( S_ST_XAX_MSTONES, xax_milestones )
        ADD_SUB_PAIR_RULE( S_ST_YAX_MSTONES, yax_milestones )
        ADD_SUB_PAIR_RULE( S_ST_LEGEND_TABLE, legend_table )
    }

    INSUB( "svg_render", "before-gsub-template" )

    if( *rc == RC_NORMAL )
    {
        template = svg->svt_chart;

        chart = gsub_string( rc, template, subs );
    }

    INSUB( "svg_render", "before-free" )

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
    struct series_data *ds, *cmax;

    if( *rc == RC_NORMAL )
    {
        ADD_SUB_PAIR_RULE( S_AXIS_RGB, strdup( svg->axis_color ) )
        allsubs = rule;
    }

    ADD_SUB_PAIR_RULE( S_BG_RGB, strdup( svg->chart_color ) )
    ADD_SUB_PAIR_RULE( S_GR_FILL_RGB, strdup( svg->graph_color ) )
    ADD_SUB_PAIR_RULE( S_SC_HI, strdup( svg->screen_height ) )
    ADD_SUB_PAIR_RULE( S_SC_WID, strdup( svg->screen_width ) )
    ADD_SUB_PAIR_RULE( S_TEXT_RGB, strdup( svg->text_color ) )
    ADD_SUB_PAIR_RULE( S_TEXT_SIZE, strdup( svg->text_size ) )
    ADD_SUB_PAIR_RULE( S_X_GRID_RGB, strdup( svg->x_gridline_color ) )
    ADD_SUB_PAIR_RULE( S_YAXIS_TITLE, strdup( svg->yax_title ) )
    ADD_SUB_PAIR_RULE( S_XAXIS_TITLE, strdup( svg->xax_title ) )
    ADD_SUB_PAIR_RULE( S_Y_GRID_RGB, strdup( svg->y_gridline_color ) )
    ADD_SUB_PAIR_RULE( S_CHA_TITLE_TEXT, strdup( svg->chart_title ) )

    ADD_SUB_PAIR_RULE( S_AXIS_OP, string_from_float( rc, svg->axis_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_AXIS_SIZE, string_from_int( rc, svg->axis_size, 0 ) )
    ADD_SUB_PAIR_RULE( S_CH_HI, string_from_int( rc, svg->chart_height, 0 ) )
    ADD_SUB_PAIR_RULE( S_CH_WID, string_from_int( rc, svg->chart_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_FILL_OP, string_from_float( rc, svg->graph_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_GR_AREA_HI, string_from_int( rc, svg->graph_height, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_AREA_WID, string_from_int( rc, svg->graph_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_LEFT_COL, string_from_int( rc, svg->graph_left_col, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_TOP_ROW, string_from_int( rc, svg->graph_top_row, 0 ) )
    ADD_SUB_PAIR_RULE( S_HI_RES, string_from_int( rc, svg->reserve_height, 0 ) )
    ADD_SUB_PAIR_RULE( S_TEXT_OP, string_from_float( rc, svg->text_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_WID_RES, string_from_int( rc, svg->reserve_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_XAXIS_WID, string_from_int( rc, svg->xax_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_X_CH_BORD, string_from_int( rc, svg->xax_border, 0 ) )
    ADD_SUB_PAIR_RULE( S_X_GRID_OP, string_from_float( rc, svg->x_gridline_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_X_GRID_SIZE, string_from_int( rc, svg->x_gridline_size, 0 ) )
    ADD_SUB_PAIR_RULE( S_YAXIS_HI, string_from_int( rc, svg->yax_height, 0 ) )
    ADD_SUB_PAIR_RULE( S_YAXIS_TEXT_COL, string_from_int( rc, svg->yax_text_col, 0 ) )
    ADD_SUB_PAIR_RULE( S_YAXIS_TEXT_FLOOR, string_from_int( rc, svg->yax_text_floor, 0 ) )
    ADD_SUB_PAIR_RULE( S_XAXIS_TEXT_FLOOR, string_from_int( rc, svg->xax_text_floor, 0 ) )
    ADD_SUB_PAIR_RULE( S_Y_CH_BORD, string_from_int( rc, svg->yax_border, 0 ) )
    ADD_SUB_PAIR_RULE( S_Y_GRID_OP, string_from_float( rc, svg->y_gridline_alpha, ALPHA_DISP_FORMAT ) )
    ADD_SUB_PAIR_RULE( S_Y_GRID_SIZE, string_from_int( rc, svg->y_gridline_size, 0 ) )
    ADD_SUB_PAIR_RULE( S_CHA_WID_MIDPOINT, string_from_int( rc, svg->chart_width_midp, 0 ) )
    ADD_SUB_PAIR_RULE( S_CHA_HI_MIDPOINT, string_from_int( rc, svg->chart_height_midp, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_WID_MIDPOINT, string_from_int( rc, svg->graph_width_midp, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_HI_MIDPOINT, string_from_int( rc, svg->graph_height_midp, 0 ) )
    ADD_SUB_PAIR_RULE( S_HDR_HI_MIDPOINT, string_from_int( rc, svg->head_height_midp, 0 ) )
    ADD_SUB_PAIR_RULE( S_GR_BOTTOM, string_from_int( rc, svg->graph_bottom, 0 ) )
    ADD_SUB_PAIR_RULE( S_MATTE_WID, string_from_int( rc, svg->matte_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_MATTE_HI, string_from_int( rc, svg->matte_height, 0 ) )

    ADD_SUB_PAIR_RULE( S_LEG_CIR_COL, string_from_int( rc, svg->leg_circ_col, 0 ) )
    ADD_SUB_PAIR_RULE( S_LEG_CONT_WID, string_from_int( rc, svg->leg_cont_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_LEG_CONT_LEFT_COL, string_from_int( rc, svg->leg_cont_left_col, 0 ) )
    ADD_SUB_PAIR_RULE( S_LEG_CONT_TOP_ROW, string_from_int( rc, svg->leg_cont_top_row, 0 ) )
    ADD_SUB_PAIR_RULE( S_LEG_FONT_SCALE, string_from_int( rc, svg->leg_font_scale, 0 ) )
    ADD_SUB_PAIR_RULE( S_LEG_LABEL_FLOOR, string_from_int( rc, svg->leg_label_floor, 0 ) )
    ADD_SUB_PAIR_RULE( S_LEG_LEFT_COL, string_from_int( rc, svg->leg_left_col, 0 ) )
    ADD_SUB_PAIR_RULE( S_LEG_LINE_LEN, string_from_int( rc, svg->leg_line_len, 0 ) )
    ADD_SUB_PAIR_RULE( S_LEG_WIDTH, string_from_int( rc, svg->leg_width, 0 ) )
    ADD_SUB_PAIR_RULE( S_LEG_SER_DESC_COL, string_from_int( rc, svg->leg_series_desc_col, 0 ) )

    /* ---
     * We need the maximum radius used for a data point in any of the defined series
     * available at the "overall" level.
     */
    cmax = svg->series;
    for( ds = svg->series; ds; ds = ds->next ) if( ds->circ_radius > cmax->circ_radius ) cmax = ds;

    ADD_SUB_PAIR_RULE( S_CIR_RAD, string_from_int( rc, cmax->circ_radius, 0 ) )
    ADD_SUB_PAIR_RULE( S_CIR_LIN_SIZE, string_from_int( rc, cmax->circ_line_size, 0 ) )

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

int svg_set_circ_fill_color( struct series_data *ds, char *val )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &ds->circ_fill_color, val );

    return( rc );
}

/* --- */

char *svg_get_circ_fill_color( struct series_data *ds )

{
    if( !ds ) return( 0 );
    else return( ds->circ_fill_color );
}

/* --- */

int svg_set_circ_line_color( struct series_data *ds, char *val )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &ds->circ_line_color, val );

    return( rc );
}

/* --- */

char *svg_get_circ_line_color( struct series_data *ds )

{
    if( !ds ) return( 0 );
    else return( ds->circ_line_color );
}

/* --- */

int svg_set_data_fill_color( struct series_data *ds, char *val )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &ds->data_fill_color, val );

    return( rc );
}

/* --- */

char *svg_get_data_fill_color( struct series_data *ds )

{
    if( !ds ) return( 0 );
    else return( ds->data_fill_color );
}

/* --- */

int svg_set_data_line_color( struct series_data *ds, char *val )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &ds->data_line_color, val );

    return( rc );
}

/* --- */

char *svg_get_data_line_color( struct series_data *ds )

{
    if( !ds ) return( 0 );
    else return( ds->data_line_color );
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

int svg_set_xax_format_type( struct svg_model *svg, int val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->xax_format_type = val;

    return( rc );
}

/* --- */

int svg_get_xax_format_type( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->xax_format_type );
}

/* --- */

int svg_set_yax_format_type( struct svg_model *svg, int val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->yax_format_type = val;

    return( rc );
}

/* --- */

int svg_get_yax_format_type( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->yax_format_type );
}

/* --- */

int svg_set_screen_width( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->screen_width, val );

    return( rc );
}

/* --- */

char *svg_get_screen_width( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->screen_width );
}

/* --- */

int svg_set_screen_height( struct svg_model *svg, char *val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &svg->screen_height, val );

    return( rc );
}

/* --- */

char *svg_get_screen_height( struct svg_model *svg )

{
    if( !svg ) return( 0 );
    else return( svg->screen_height );
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

int svg_set_data_line_size( struct series_data *ds, int size )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else ds->data_line_size = size;

    return( rc );
}

/* --- */

int svg_get_data_line_size( struct series_data *ds)

{
    if( !ds ) return( 0 );
    else return( ds->data_line_size );
}

/* --- */

int svg_set_circ_line_size( struct series_data *ds, int size )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else ds->circ_line_size = size;

    return( rc );
}

/* --- */

int svg_get_circ_line_size( struct series_data *ds )

{
    if( !ds ) return( 0 );
    else return( ds->circ_line_size );
}

/* --- */

int svg_set_circ_radius( struct series_data *ds, int size )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else ds->circ_radius = size;

    return( rc );
}

/* --- */

int svg_get_circ_radius( struct series_data *ds )

{
    if( !ds ) return( 0 );
    else return( ds->circ_radius );
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

int svg_set_circ_fill_alpha( struct series_data *ds, float val )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else ds->circ_fill_alpha = val;

    return( rc );
}

/* --- */

float svg_get_circ_fill_alpha( struct series_data *ds )

{
    if( !ds ) return( 0.0 );
    else return( ds->circ_fill_alpha );
}

/* --- */

int svg_set_circ_line_alpha( struct series_data *ds, float val )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else ds->circ_line_alpha = val;

    return( rc );
}

/* --- */

float svg_get_circ_line_alpha( struct series_data *ds )

{
    if( !ds ) return( 0.0 );
    else return( ds->circ_line_alpha );
}

/* --- */

int svg_set_data_fill_alpha( struct series_data *ds, float val )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else ds->data_fill_alpha = val;

    return( rc );
}

/* --- */

float svg_get_data_fill_alpha( struct series_data *ds )

{
    if( !ds ) return( 0.0 );
    else return( ds->data_fill_alpha );
}

/* --- */

int svg_set_data_line_alpha( struct series_data *ds, float val )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else ds->data_line_alpha = val;

    return( rc );
}

/* --- */

float svg_get_data_line_alpha( struct series_data *ds )

{
    if( !ds ) return( 0.0 );
    else return( ds->data_line_alpha );
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


/* --- */

int svg_set_chart_width( struct svg_model *svg, int val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->chart_width = val;

    return( rc );
}

/* --- */

int svg_get_chart_width( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->chart_width );
}



/* --- */

int svg_set_chart_height( struct svg_model *svg, int val )

{
    int rc = RC_NORMAL;

    if( !svg ) rc = ERR_UNSUPPORTED;
    else svg->chart_height = val;

    return( rc );
}

/* --- */

int svg_get_chart_height( struct svg_model *svg )

{
    if( !svg ) return( 0.0 );
    else return( svg->chart_height );
}


/* --- */

struct svg_chart_milestone *svg_add_xax_checkpoint( struct svg_model *svg, float offset,
  char *label )

{
    struct svg_chart_milestone *curr, *first, *last;

    curr = (struct svg_chart_milestone *) malloc( sizeof *curr );
    if( curr )
    {
        curr->offset = offset;
        curr->next = 0;
        curr->width = SVG_NO_VALUE;
        curr->extend = SVG_NO_VALUE;
        curr->text_alpha = SVG_NO_VALUE;
        curr->line_alpha = SVG_NO_VALUE;
        curr->text_size = 0;
        curr->text_color = 0;
        curr->line_color = 0;

        if( !label ) curr->label = 0;
        else if( !*label ) curr->label = 0;
        else
        {
            curr->label = strdup( label );
            if( !curr->label)
            {
                free( curr );
                curr = 0;
            }
	}
    }

    if( curr )
    {
        first = svg->xmiles;
        if( first )
        {
            curr->width = first->width;
            curr->extend = first->extend;
            curr->text_alpha = first->text_alpha;
            curr->line_alpha = first->line_alpha;
            if( first->text_size) curr->text_size = strdup( first->text_size );
            else curr->text_size = 0;
            if( first->text_color) curr->text_color = strdup( first->text_color );
            else curr->text_color = 0;
            if( first->line_color) curr->line_color = strdup( first->line_color );
            else curr->line_color = 0;
	    for( last = svg->xmiles; last; last = last->next)
            {
                if( !last->next )
                {
                    last->next = curr;
                    last = curr;
		}
	    }
	}
        else svg->xmiles = curr;
    }

    return( curr );
}

/* --- */

struct svg_chart_milestone *svg_add_yax_checkpoint( struct svg_model *svg, float offset,
  char *label )

{
    struct svg_chart_milestone *curr, *first, *last;

    curr = (struct svg_chart_milestone *) malloc( sizeof *curr );
    if( curr )
    {
        curr->offset = offset;
        curr->next = 0;
        curr->label = strdup( label );
        if( !curr->label)
        {
            free( curr );
            curr = 0;
	}
    }

    if( curr )
    {
        first = svg->ymiles;
        if( first )
        {
            curr->width = first->width;
            curr->extend = first->extend;
            curr->text_alpha = first->text_alpha;
            curr->line_alpha = first->line_alpha;
            curr->text_color = first->text_color;
            curr->line_color = first->line_color;
	    for( last = svg->ymiles; last->next; last = last->next )
            last->next = curr;
	}
        else
        {
            curr->width = 0;
            curr->extend = curr->text_alpha = curr->line_alpha = 0.0;
            curr->text_color = curr->line_color = 0;
            svg->ymiles = curr;
	}
    }

    return( curr );
}

/* --- */

void svg_set_checkpoint_width( struct svg_chart_milestone *ckpt, int width )

{
    ckpt->width = width;

    return;
}

/* --- */

void svg_set_checkpoint_extend( struct svg_chart_milestone *ckpt, int extend )

{
    ckpt->extend = extend;

    return;
}

/* --- */

void svg_set_checkpoint_text_alpha( struct svg_chart_milestone *ckpt, float text_alpha )

{
    ckpt->text_alpha = text_alpha;

    return;
}

/* --- */

void svg_set_checkpoint_line_alpha( struct svg_chart_milestone *ckpt, float line_alpha )

{
    ckpt->line_alpha = line_alpha;

    return;
}

/* --- */

int svg_set_checkpoint_text_color( struct svg_chart_milestone *ckpt, char *text_color )

{
    int rc = RC_NORMAL;

    if( ckpt->text_color ) free( ckpt->text_color );
    ckpt->text_color = strdup( text_color );
    if( !ckpt->text_color ) rc = ERR_MALLOC_FAILED;

    return( rc );
}

/* --- */

int svg_set_checkpoint_line_color( struct svg_chart_milestone *ckpt, char *line_color )

{
    int rc = RC_NORMAL;

    if( ckpt->line_color ) free( ckpt->line_color );
    ckpt->line_color = strdup( line_color );
    if( !ckpt->line_color ) rc = ERR_MALLOC_FAILED;

    return( rc );
}

/* --- */

int svg_set_checkpoint_text_size( struct svg_chart_milestone *ckpt, char *text_size )

{
    int rc = RC_NORMAL;
    if( ckpt->text_size ) free( ckpt->text_size );
    ckpt->text_size = strdup( text_size );
    if( !ckpt->text_size ) rc = ERR_MALLOC_FAILED;

    return( rc );
}

/* --- */

int svg_get_checkpoint_width( struct svg_chart_milestone *ckpt )

{   return( ckpt->width ); }

/* --- */

int svg_get_checkpoint_extend( struct svg_chart_milestone *ckpt )

{   return( ckpt->extend ); }

/* --- */

float svg_get_checkpoint_text_alpha( struct svg_chart_milestone *ckpt )

{   return( ckpt->text_alpha ); }

/* --- */

float svg_get_checkpoint_line_alpha( struct svg_chart_milestone *ckpt )

{   return( ckpt->line_alpha ); }

/* --- */

char *svg_get_checkpoint_text_color( struct svg_chart_milestone *ckpt )

{   return( ckpt->text_color ); }

/* --- */

char *svg_get_checkpoint_line_color( struct svg_chart_milestone *ckpt )

{   return( ckpt->line_color ); }

/* --- */

char *svg_get_checkpoint_text_size( struct svg_chart_milestone *ckpt )

{   return( ckpt->text_size ); }

/* --- */

int svg_get_legend_scale( struct svg_model *svg )

{   return( svg->legend_scale ); }

/* --- */

int svg_get_has_legend( struct svg_model *svg )

{   return( svg->has_legend ); }

/* --- */

void svg_set_legend_scale( struct svg_model *svg, int val )

{
    svg->legend_scale = val;
    return;
}

/* --- */

void svg_set_has_legend( struct svg_model *svg, int val )

{
    svg->has_legend = !!val;
    return;

}

/* --- */

char *svg_make_xax_mstones( int *rc, struct svg_model *svg )

{
    int pad, nstones, xax_coord, seq = 0, label_yax_hi, label_ypos;
    char *line_frag = 0, *label_frag = 0, *result = 0, *seg = 0,
      *line_patt = 0, *label_patt = 0, *combo;
    struct svg_chart_milestone *walk;
    struct sub_list *subs, *last, *line_color = 0, *line_size = 0,
      *line_alpha = 0, *line_width = 0, *text_color = 0, *text_size = 0,
      *text_alpha = 0, *label = 0, *line_xpos = 0, *line_ypos = 0,
      *text_xpos = 0, *text_ypos = 0;

    for( walk = svg->xmiles, nstones = 0; walk; walk = walk->next ) nstones++;

    if( !nstones ) result = strdup( "\n" );
    else
    {
    
        last = subs = (struct sub_list *) malloc( sizeof *subs );
        if( last )
        {
            line_color = last;
            line_color->from = S_MST_LINE_COLOR;
            last = (struct sub_list *) malloc( sizeof *last );
            line_color->next = last;
        }
        if( last )
        {
            line_size = last;
            line_size->from = S_MST_LINE_SIZE;
            last = (struct sub_list *) malloc( sizeof *last );
            line_size->next = last;
        }
        if( last )
        {
            line_alpha = last;
            line_alpha->from = S_MST_LINE_ALPHA;
            last = (struct sub_list *) malloc( sizeof *last );
            line_alpha->next = last;
        }
        if( last )
        {
            line_width = last;
            line_width->from = S_MST_LINE_WIDTH;
            last = (struct sub_list *) malloc( sizeof *last );
            line_width->next = last;
        }
        if( last )
        {
            text_color = last;
            text_color->from = S_MST_TEXT_COLOR;
            last = (struct sub_list *) malloc( sizeof *last );
            text_color->next = last;
        }
        if( last )
        {
            text_size = last;
            text_size->from = S_MST_TEXT_SIZE;
            last = (struct sub_list *) malloc( sizeof *last );
            text_size->next = last;
        }
        if( last )
        {
            text_alpha = last;
            text_alpha->from = S_MST_TEXT_ALPHA;
            last = (struct sub_list *) malloc( sizeof *last );
            text_alpha->next = last;
        }
        if( last )
        {
            label = last;
            label->from = S_MST_LABEL;
            last = (struct sub_list *) malloc( sizeof *last );
            label->next = last;
        }
        if( last )
        {
            line_xpos = last;
            line_xpos->from = S_MST_LINE_XPOS;
            last = (struct sub_list *) malloc( sizeof *last );
            line_xpos->next = last;
        }
        if( last )
        {
            line_ypos = last;
            line_ypos->from = S_MST_LINE_YPOS;
            last = (struct sub_list *) malloc( sizeof *last );
            line_ypos->next = last;
        }
        if( last )
        {
            text_xpos = last;
            text_xpos->from = S_MST_TEXT_XPOS;
            last = (struct sub_list *) malloc( sizeof *last );
            text_xpos->next = last;
        }
        if( last )
        {
            text_ypos = last;
            text_ypos->from = S_MST_TEXT_YPOS;
            text_ypos->next = 0;
        }

        if( !text_ypos) *rc = ERR_MALLOC_FAILED;

        line_patt = SVG_XSTONE_LINE;
        label_patt = SVG_XSTONE_LABEL;
        label_yax_hi = (svg->yax_border - svg->graph_top_row) / (nstones + 2);
        label_ypos = svg->graph_top_row + (label_yax_hi / 2);

        if( *rc == RC_NORMAL ) for( walk = svg->xmiles; walk; walk = walk->next, seq++ )
        {
            text_size->to = walk->text_size;
            text_color->to = walk->text_color;
            line_color->to = walk->line_color;
            label->to = walk->label;

            text_alpha->to = string_from_float( rc, walk->text_alpha, ALPHA_DISP_FORMAT );
            line_alpha->to = string_from_float( rc, walk->line_alpha, ALPHA_DISP_FORMAT );
            line_width->to = string_from_int( rc, walk->width, 0 );

            pad = (int) ((float) svg->graph_height * walk->extend);
            line_size->to = string_from_int( rc, svg->graph_height + (pad*2), 0 );

            xax_coord = svg_xax_raw_to_gc( svg, walk->offset );
            line_xpos->to = string_from_int( rc, xax_coord, 0 );
            line_ypos->to = string_from_int( rc, svg->yax_border + pad, 0 );
            text_xpos->to = string_from_int( rc, xax_coord - walk->width, 0 );
            text_ypos->to = string_from_int( rc, label_ypos, 0 );
            label_ypos += label_yax_hi;

            if( *rc == RC_NORMAL ) seg = gsub_string( rc, line_patt, subs );
            combo = combine_strings( rc, line_frag, seg );
            if( line_frag ) free( line_frag );
            if( seg ) free( seg );
            line_frag = combo;

            if( *rc == RC_NORMAL ) seg = gsub_string( rc, label_patt, subs );
            combo = combine_strings( rc, label_frag, seg );
            if( label_frag ) free( label_frag );
            if( seg ) free( seg );
            label_frag = combo;

            if( text_alpha->to ) free( text_alpha->to );
            if( line_alpha->to ) free( line_alpha->to );
            if( line_width->to ) free( line_width->to );
            if( line_size->to ) free( line_size->to );
            if( line_xpos->to ) free( line_xpos->to );
            if( line_ypos->to ) free( line_ypos->to );
            if( text_xpos->to ) free( text_xpos->to );
            if( text_ypos->to ) free( text_ypos->to );
        }

        result = combine_strings( rc, line_frag, label_frag );
        if( line_frag ) free( line_frag );
        if( label_frag ) free( label_frag );

        for( last = subs; last; )
        {
            subs = last;
            last = last->next;
            free( subs );
	}
    }
 
    return( result );
}

/* --- */

char *svg_make_yax_mstones( int *rc, struct svg_model *svg )

{
    return( strdup( "<!-- yax-milestones stubbed out for now -->"));
}


/* --- */

int svg_xax_raw_to_gc( struct svg_model *svg, float offset )

{
    int coord, scale;
    float offset_pc;

    scale = svg->xax_border - svg->graph_left_col;
    offset_pc = (offset - svg->xmin) / (svg->xmax - svg->xmin);
    coord = (int) (svg->graph_left_col + ((float) scale * offset_pc));
    
    return( coord );
}

/* --- */

int svg_set_data_desc( struct series_data *ds, char *val )

{
    int rc = RC_NORMAL;

    if( !ds ) rc = ERR_UNSUPPORTED;
    else if( !val ) rc = ERR_BAD_PARMS;
    else rc = svg_replace_string( &ds->descr, val );

    return( rc );
}

/* --- */

char *svg_get_data_desc( struct series_data *ds )

{
    if( !ds ) return( 0 );
    else return( ds->descr );
}

