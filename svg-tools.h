#ifndef ___CNAMEjj_SVG_TOOLS_H__

#define ___CNAMEjj_SVG_TOOLS_H__

#include "cli-sub.h"

/* --- */

#pragma GCC diagnostic ignored "-Woverlength-strings"

#define SVG_NO_VALUE -999999

#define DEF_AXIS_RGB "#FFFFFF"
#define DEF_AXIS_OP 0.6
#define DEF_BG_RGB "#302716"
#define DEF_BG_OP 1.0
#define DEF_CIR_FILL_RGB "#32DE48"
#define DEF_CIR_FILL_OP 0.2
#define DEF_CIR_LIN_RGB "#F3E0F8"
#define DEF_CIR_LIN_OP 0.7
#define DEF_DAT_FILL_RGB "#FFFFFF"
#define DEF_DAT_FILL_OP 0.0
#define DEF_DAT_LIN_RGB "#F4E6EE"
#define DEF_DAT_LIN_OP 0.5
#define DEF_GR_FILL_RGB "#E034FF"
#define DEF_GR_FILL_OP 0.05
#define DEF_SC_HI "50%"
#define DEF_SC_WID "100%"
#define DEF_TEXT_RGB "#A89236"
#define DEF_TEXT_OP 1.0
#define DEF_X_GRID_RGB "#4DDFA2"
#define DEF_X_GRID_OP 0.2
#define DEF_Y_GRID_RGB "#4DDFA2"
#define DEF_Y_GRID_OP 0.2
#define DEF_YAXIS_TITLE ""
#define DEF_XAXIS_TITLE ""
#define DEF_CHA_TITLE_TEXT ""
#define DEF_CH_HI 900
#define DEF_CH_WID 3000
/* Text size is a string, since it can be things like "80%", but the
 * default of an empty string triggers code that calculates it from
 * the chart height.
 */
#define DEF_TEXT_SIZE ""
#define DEF_XAXIS_DIS_FORMAT "%.3f"
#define DEF_YAXIS_DIS_FORMAT "%.1f"

#define S_AXIS_RGB "++axis-color++"
#define S_AXIS_OP "++axis-opacity++"
#define S_AXIS_SIZE "++axis-size++"
#define S_BG_RGB "++bg-color++"
#define S_CH_HI "++chart-height++"
#define S_CH_WID "++chart-width++"
#define S_CIR_FILL_RGB "++circle-fill-color++"
#define S_CIR_FILL_OP "++circle-fill-opacity++"
#define S_CIR_LIN_RGB "++circle-line-color++"
#define S_CIR_LIN_OP "++circle-line-opacity++"
#define S_CIR_LIN_SIZE "++circle-line-size++"
#define S_CIR_RAD "++circle-radius++"
#define S_DAT_FILL_RGB "++data-fill-color++"
#define S_DAT_FILL_OP "++data-fill-opacity++"
#define S_DAT_LIN_RGB "++data-line-color++"
#define S_DAT_LIN_OP "++data-line-opacity++"
#define S_DAT_LIN_SIZE "++data-line-size++"
#define S_GR_FILL_RGB "++chart-bg-color++"
#define S_GR_FILL_OP "++chart-bg-opacity++"
#define S_GR_AREA_HI "++graph-area-height++"
#define S_GR_AREA_WID "++graph-area-width++"
#define S_GR_LEFT_COL "++graph-left-col++"
#define S_GR_TOP_ROW "++graph-top-row++"
#define S_HI_RES "++height-reserve++"
#define S_SC_HI "++screen-height++"
#define S_SC_WID "++screen-width++"
#define S_TEXT_RGB "++text-color++"
#define S_TEXT_OP "++text-opacity++"
#define S_TEXT_SIZE "++text-size++"
#define S_WID_RES "++width-reserve++"
#define S_XAXIS_WID "++x-axis-width++"
#define S_X_CH_BORD "++x-chart-border++"
#define S_X_GRID_RGB "++x-gridline-color++"
#define S_X_GRID_OP "++x-gridline-opacity++"
#define S_X_GRID_SIZE "++x-gridline-size++"
#define S_YAXIS_HI "++y-axis-height++"
#define S_YAXIS_TEXT_COL "++y-axis-text-col++"
#define S_YAXIS_TEXT_FLOOR "++y-axis-text-floor++"
#define S_YAXIS_TITLE "++y-axis-title++"
#define S_XAXIS_TEXT_FLOOR "++x-axis-text-floor++"
#define S_XAXIS_TITLE "++x-axis-title++"
#define S_Y_CH_BORD "++y-chart-border++"
#define S_Y_GRID_RGB "++y-gridline-color++"
#define S_Y_GRID_OP "++y-gridline-opacity++"
#define S_Y_GRID_SIZE "++y-gridline-size++"
#define S_CHA_TITLE_TEXT "++chart-title++"
#define S_CHA_WID_MIDPOINT "++chart-width-midpoint++"
#define S_CHA_HI_MIDPOINT "++chart-height-midpoint++"
#define S_GR_WID_MIDPOINT "++graph-width-midpoint++"
#define S_GR_HI_MIDPOINT "++graph-height-midpoint++"
#define S_HDR_HI_MIDPOINT "++header-height-midpoint++"
#define S_GR_BOTTOM "++graph-bottom++"

#define S_XPOS "++xpos++"
#define S_YPOS "++ypos++"
#define S_LABEL "++label++"

#define S_ST_ROW_LABEL "++row-label++"
#define S_ST_ROW_LINE "++row-line++"
#define S_ST_COL_LABEL "++col-label++"
#define S_ST_COL_LINE "++col-line++"
#define S_ST_DATA_POINT "++circ-elem++"
#define S_ST_START_PATH "++path-start++"
#define S_ST_DATA_LINE "++path-points++"

/* --- */

#define SVG_ROW_LABEL   "      <tspan x=\"++y-axis-text-col++\" dy=\"-++y-axis-height++\">++label++</tspan>\n"
#define SVG_ROW_LINE    "        m -++graph-area-width++ -++y-axis-height++ h ++graph-area-width++\n"
#define SVG_COL_LABEL   "      <tspan x=\"++xpos++\" y=\"++x-axis-text-floor++\">++label++</tspan>\n"
#define SVG_COL_LINE    "      m ++x-axis-width++ -++graph-area-height++ v ++graph-area-height++\n"
#define SVG_CIRC_ELEM   "      <circle cx=\"++xpos++\" cy=\"++ypos++\" r=\"++circle-radius++\"/>\n"
#define SVG_PATH_START  "      M ++xpos++ ++ypos++ L"
#define SVG_PATH_POINTS "  ++xpos++ ++ypos++"

#define SVG_CHART_TEMPLATE "\
<svg version=\"1.1\" baseProfile=\"full\"\n\
  width=\"++screen-width++\" height=\"++screen-height++\"\n\
  viewbox=\"0 0 ++chart-width++ ++chart-height++\"\n\
  xmlns=\"http://www.w3.org/2000/svg\">\n\
\n\
  <rect width=\"100%\" height=\"100%\" fill=\"++bg-color++\" />\n\
\n\
  <path d=\"M ++graph-left-col++ ++graph-top-row++ v ++graph-area-height++ h ++graph-area-width++\"\n\
    stroke=\"++axis-color++\" stroke-opacity=\"++axis-opacity++\" fill=\"transparent\"\n\
    stroke-width=\"++axis-size++\" />\n\
\n\
  <path d=\"M ++graph-left-col++ ++graph-top-row++ v ++graph-area-height++ h ++graph-area-width++ v -++graph-area-height++\"\n\
    stroke-opacity=\"0\" fill=\"++chart-bg-color++\" fill-opacity=\"++chart-bg-opacity++\"\n\
    stroke-width=\"++axis-size++\" />\n\
\n\
  <g fill=\"++text-color++\" font-size=\"++text-size++\" fill-opacity=\"++text-opacity++\" style=\"font-family:Verdana, Arial, Helvetica, sans-serif; font-weight:100\">\n\
    <text style=\"font-size:150%\">\n\
      <tspan x=\"++chart-width-midpoint++\" y=\"++header-height-midpoint++\"\n\
        style=\"dominant-baseline:central; font-weight:100\" text-anchor=\"middle\">++chart-title++</tspan>\n\
    </text>\n\
\n\
    <text x=\"++width-reserve++\" y=\"++graph-height-midpoint++\" text-anchor=\"middle\"\n\
      style=\"dominant-baseline:text-after-edge\"\n\
      transform=\"rotate(-90, ++width-reserve++, ++graph-height-midpoint++)\">++y-axis-title++</text>\n\
\n\
    <text x=\"++graph-width-midpoint++\" y=\"++graph-bottom++\" text-anchor=\"middle\"\n\
      style=\"dominant-baseline:hanging\">++x-axis-title++</text>\n\
\n\
    <text x=\"++y-axis-text-col++\" y=\"++y-axis-text-floor++\" style=\"dominant-baseline:central\" text-anchor=\"end\">\n\
++row-label++\n\
      <tspan x=\"0\" y=\"0\" stroke=\"transparent\">.</tspan>\n\
    </text>\n\
\n\
    <g stroke=\"++x-gridline-color++\" stroke-width=\"++x-gridline-size++\" stroke-opacity=\"++x-gridline-opacity++\">\n\
      <path d=\"M ++x-chart-border++ ++y-chart-border++\n\
++row-line++\n\
        \"/>\n\
    </g>\n\
\n\
    <text text-anchor=\"middle\" style=\"dominant-baseline:text-before-edge\">\n\
++col-label++\n\
    </text>\n\
  </g>\n\
\n\
  <g stroke=\"++y-gridline-color++\" stroke-width=\"++y-gridline-size++\" stroke-opacity=\"++y-gridline-opacity++\">\n\
    <path d=\"M ++graph-left-col++ ++y-chart-border++\n\
++col-line++\n\
      \"/>\n\
  </g>\n\
\n\
  <g fill=\"++circle-fill-color++\" stroke=\"++circle-line-color++\" stroke-width=\"++circle-line-size++\"\n\
    stroke-opacity=\"++circle-line-opacity++\" fill-opacity=\"++circle-fill-opacity++\">\n\
++circ-elem++\n\
  </g>\n\
\n\
  <path d=\"\n\
++path-start++\n\
++path-points++\n\
    \"\n\
    stroke=\"++data-line-color++\" stroke-opacity=\"++data-line-opacity++\" stroke-width=\"++data-line-size++\"\n\
    fill=\"++data-fill-color++\" fill-opacity=\"++data-fill-opacity++\"/>\n\
\n\
</svg>"

/* --- */

struct svg_model
{
    int axis_size, xax_num_grids, yax_num_grids,
      xax_border, xax_text_floor, xax_width, xax_text_adj, x_gridline_size,
      yax_border, yax_text_col, yax_text_floor, yax_height, yax_text_adj, y_gridline_size,
      graph_left_col, graph_width, graph_height, graph_top_row,
      graph_bottom, graph_height_midp, graph_width_midp,
      chart_height, chart_width_midp, chart_height_midp, chart_width,
      head_height_midp,
      circ_line_size, circ_radius,
      data_line_size,
      reserve_height, reserve_width,
      shift_width, shift_height, shift_bottom,
      cases;

    float axis_alpha, graph_alpha, chart_alpha, text_alpha,
      circ_fill_alpha, circ_line_alpha,
      data_fill_alpha, data_line_alpha,
      x_gridline_alpha, y_gridline_alpha;

    double xmin, xmax, ymin, ymax, *xdata, *ydata;

    char *chart_title, *xax_title, *yax_title, *text_size,
      *axis_color, *chart_color, *graph_color, *circ_fill_color,
      *circ_line_color, *data_fill_color, *data_line_color, *text_color,
      *x_gridline_color, *y_gridline_color, *xax_disp, *yax_disp,
      *screen_height, *screen_width, 
      *svt_row_label, *svt_row_line, *svt_col_label, *svt_col_line,
      *svt_circ_elem, *svt_path_start, *svt_path_points, *svt_chart;

};

/* --- */

struct svg_model *svg_make_chart();

void svg_free_model(struct svg_model *svg);

int svg_finalize_model(struct svg_model *svg);

int svg_add_double_data(struct svg_model *svg, int cases, double *xval, double *yval);

int svg_add_float_data(struct svg_model *svg, int cases, float *xval, float *yval);

int svg_add_int_data(struct svg_model *svg, int cases, int *xval, int *yval);

char *svg_make_yax_labels(int *rc, struct svg_model *svg);

char *svg_make_xax_labels(int *rc, struct svg_model *svg);

char *svg_make_xax_grid(int *rc, struct svg_model *svg);

char *svg_make_yax_grid(int *rc, struct svg_model *svg);

char *svg_make_data_points(int *rc, struct svg_model *svg);

char *svg_make_path_start(int *rc, struct svg_model *svg);

char *svg_make_data_lines(int *rc, struct svg_model *svg);

struct sub_list *svg_make_sublist(int *rc, struct svg_model *svg);

char *svg_render(int *rc, struct svg_model *svg);

int svg_set_chart_title(struct svg_model *svg, char *title);
int svg_set_xax_title(struct svg_model *svg, char *title);
int svg_set_yax_title(struct svg_model *svg, char *title);
int svg_set_text_size(struct svg_model *svg, char *val);
int svg_set_text_color(struct svg_model *svg, char *val);
int svg_set_axis_color(struct svg_model *svg, char *val);
int svg_set_chart_color(struct svg_model *svg, char *val);
int svg_set_graph_color(struct svg_model *svg, char *val);
int svg_set_circ_fill_color(struct svg_model *svg, char *val);
int svg_set_circ_line_color(struct svg_model *svg, char *val);
int svg_set_data_fill_color(struct svg_model *svg, char *val);
int svg_set_data_line_color(struct svg_model *svg, char *val);
int svg_set_x_gridline_color(struct svg_model *svg, char *val);
int svg_set_y_gridline_color(struct svg_model *svg, char *val);
int svg_set_xax_disp(struct svg_model *svg, char *val);
int svg_set_yax_disp(struct svg_model *svg, char *val);
int svg_set_xax_num_grids(struct svg_model *svg, int size);
int svg_set_yax_num_grids(struct svg_model *svg, int size);
int svg_set_reserve_height(struct svg_model *svg, int size);
int svg_set_reserve_width(struct svg_model *svg, int size);
int svg_set_shift_height(struct svg_model *svg, int size);
int svg_set_shift_width(struct svg_model *svg, int size);
int svg_set_shift_bottom(struct svg_model *svg, int size);
int svg_set_data_line_size(struct svg_model *svg, int size);
int svg_set_circ_line_size(struct svg_model *svg, int size);
int svg_set_circ_radius(struct svg_model *svg, int size);
int svg_set_x_gridline_size(struct svg_model *svg, int size);
int svg_set_y_gridline_size(struct svg_model *svg, int size);
int svg_set_xax_text_adj(struct svg_model *svg, int size);
int svg_set_yax_text_adj(struct svg_model *svg, int size);
int svg_set_axis_size(struct svg_model *svg, int size);
int svg_set_axis_alpha(struct svg_model *svg, float val);
int svg_set_graph_alpha(struct svg_model *svg, float val);
int svg_set_chart_alpha(struct svg_model *svg, float val);
int svg_set_text_alpha(struct svg_model *svg, float val);
int svg_set_circ_fill_alpha(struct svg_model *svg, float val);
int svg_set_circ_line_alpha(struct svg_model *svg, float val);
int svg_set_data_fill_alpha(struct svg_model *svg, float val);
int svg_set_data_line_alpha(struct svg_model *svg, float val);
int svg_set_x_gridline_alpha(struct svg_model *svg, float val);
int svg_set_y_gridline_alpha(struct svg_model *svg, float val);
int svg_set_xmin(struct svg_model *svg, double val);
int svg_set_xmax(struct svg_model *svg, double val);
int svg_set_ymin(struct svg_model *svg, double val);
int svg_set_ymax(struct svg_model *svg, double val);

char *svg_get_chart_title(struct svg_model *svg);
char *svg_get_xax_title(struct svg_model *svg);
char *svg_get_yax_title(struct svg_model *svg);
char *svg_get_text_size(struct svg_model *svg);
char *svg_get_text_color(struct svg_model *svg);
char *svg_get_axis_color(struct svg_model *svg);
char *svg_get_chart_color(struct svg_model *svg);
char *svg_get_graph_color(struct svg_model *svg);
char *svg_get_circ_fill_color(struct svg_model *svg);
char *svg_get_circ_line_color(struct svg_model *svg);
char *svg_get_data_fill_color(struct svg_model *svg);
char *svg_get_data_line_color(struct svg_model *svg);
char *svg_get_x_gridline_color(struct svg_model *svg);
char *svg_get_y_gridline_color(struct svg_model *svg);
char *svg_get_xax_disp(struct svg_model *svg);
char *svg_get_yax_disp(struct svg_model *svg);
int svg_get_xax_num_grids(struct svg_model *svg);
int svg_get_yax_num_grids(struct svg_model *svg);
int svg_get_reserve_height(struct svg_model *svg);
int svg_get_reserve_width(struct svg_model *svg);
int svg_get_shift_height(struct svg_model *svg);
int svg_get_shift_width(struct svg_model *svg);
int svg_get_shift_bottom(struct svg_model *svg);
int svg_get_data_line_size(struct svg_model *svg);
int svg_get_circ_line_size(struct svg_model *svg);
int svg_get_circ_radius(struct svg_model *svg);
int svg_get_x_gridline_size(struct svg_model *svg);
int svg_get_y_gridline_size(struct svg_model *svg);
int svg_get_xax_text_adj(struct svg_model *svg);
int svg_get_yax_text_adj(struct svg_model *svg);
int svg_get_axis_size(struct svg_model *svg);
float svg_get_axis_alpha(struct svg_model *svg);
float svg_get_graph_alpha(struct svg_model *svg);
float svg_get_chart_alpha(struct svg_model *svg);
float svg_get_text_alpha(struct svg_model *svg);
float svg_get_circ_fill_alpha(struct svg_model *svg);
float svg_get_circ_line_alpha(struct svg_model *svg);
float svg_get_data_fill_alpha(struct svg_model *svg);
float svg_get_data_line_alpha(struct svg_model *svg);
float svg_get_x_gridline_alpha(struct svg_model *svg);
float svg_get_y_gridline_alpha(struct svg_model *svg);
double svg_get_xmin(struct svg_model *svg);
double svg_get_xmax(struct svg_model *svg);
double svg_get_ymin(struct svg_model *svg);
double svg_get_ymax(struct svg_model *svg);


#endif