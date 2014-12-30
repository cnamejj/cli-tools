#ifndef ___CNAMEjj_SVG_IT_H__

#define ___CNAMEjj_SVG_IT_H__

/* --- */

#define TASK_PARSE_COMMAND      "Parse command line options"
#define TASK_LOAD_DATA          "Load dataset"
#define TASK_ALLOC_CHART_OBJECT "Allocating chart object"
#define TASK_CONFIGURE_CHART    "Set chart display parameters"
#define TASK_RENDER_CHART       "Render chart to SVG document"
#define TASK_WRITE_SVG_DOC      "Write out SVG document"
#define TASK_OPEN_OUTPUT_FILE   "Open SVG document for writing"

#define DO_PARSE_COMMAND      1
#define DO_LOAD_DATA          2
#define DO_ALLOC_CHART_OBJECT 3
#define DO_CONFIGURE_CHART    4
#define DO_RENDER_CHART       5
#define DO_WRITE_SVG_DOC      6
#define DO_OPEN_OUTPUT_FILE   7

#ifdef ALLOC_CONTEXT_DATA

struct context_info {
    int ref;
    char *desc;
};

static struct context_info context_list[] = {
    {DO_PARSE_COMMAND, TASK_PARSE_COMMAND},
    {DO_LOAD_DATA, TASK_LOAD_DATA},
    {DO_ALLOC_CHART_OBJECT, TASK_ALLOC_CHART_OBJECT},
    {DO_CONFIGURE_CHART, TASK_CONFIGURE_CHART},
    {DO_RENDER_CHART, TASK_RENDER_CHART},
    {DO_WRITE_SVG_DOC, TASK_WRITE_SVG_DOC},
    {DO_OPEN_OUTPUT_FILE, TASK_OPEN_OUTPUT_FILE},
};

#endif

#define OUT_OPEN_FLAGS O_CREAT | O_TRUNC | O_WRONLY
#define OUT_OPEN_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

#define DATA_OPEN_FLAGS O_RDONLY

#define OP_CHART_TITLE 0
#define OP_XAX_TITLE   1
#define OP_YAX_TITLE   2
#define OP_XAX_GRIDS   3
#define OP_YAX_GRIDS   4
#define OP_OUTFILE     5
#define OP_DEBUG       6
#define OP_HELP        7
#define OP_DATAFILE    8
#define OP_XCOL        9
#define OP_YCOL        10
#define OP_XDATA       11
#define OP_YDATA       12

#define FL_CHART_TITLE "title"
#define FL_XAX_TITLE   "xtitle"
#define FL_YAX_TITLE   "ytitle"
#define FL_XAX_GRIDS   "xgrids"
#define FL_YAX_GRIDS   "ygrids"
#define FL_OUTFILE     "out"
#define FL_DEBUG       "debug"
#define FL_HELP        "help"
#define FL_DATAFILE    "data"
#define FL_XCOL        "xcol"
#define FL_YCOL        "ycol"
#define FL_XDATA       "xdata"
#define FL_YDATA       "ydata"

#define DEF_CHART_TITLE ""
#define DEF_XAX_TITLE   ""
#define DEF_YAX_TITLE   ""
#define DEF_XAX_GRIDS   "0"
#define DEF_YAX_GRIDS   "0"
#define DEF_OUTFILE     ""
#define DEF_DEBUG       "0"
#define DEF_HELP        "0"
#define DEF_DATAFILE    "svg.data"
#define DEF_XCOL        "1"
#define DEF_YCOL        "2"
#define DEF_XDATA       "1"
#define DEF_YDATA       "1"

#define IS_STDIN "-"
#define IS_STDOUT "-"

#define DATABUFFSIZE 8192

#define LABEL_META_FORMAT "%%.%df"

#define SC_XAX_GRIDS 7
#define SC_YAX_GRIDS 5
#define SC_CIRC_LINE_SIZE 10
#define SC_CIRC_RADIUS 22
#define SC_TEXT_COLOR "#124488"
#define SC_AXIS_COLOR "#444444"
#define SC_CHART_COLOR "#FFFFFF"
#define SC_GRAPH_COLOR "#000000"
#define SC_CIRC_FILL_COLOR "#088ea0"
#define SC_CIRC_LINE_COLOR "#e0da24"
#define SC_DATA_FILL_COLOR "#FFFFFF"
#define SC_DATA_LINE_COLOR "#744e18"
#define SC_XGRID_COLOR "#2e2e2e"
#define SC_YGRID_COLOR "#2e2e2e"
#define SC_GRAPH_ALPHA 0.05
#define SC_CIRC_FILL_ALPHA 0.3
#define SC_CIRC_LINE_ALPHA 0.4
#define SC_DATA_FILL_ALPHA 0.0
#define SC_DATA_LINE_ALPHA 0.6

/* --- */

struct parsed_options {
    int debug, help, xax_grids, yax_grids, x_col, y_col, x_data, y_data;
    char *data_file, *out_file, *chart_title, *xax_title, *yax_title;
};

struct data_pair_list {
    int cases;
    float *xval, *yval;
};

struct data_block_list {
    int size;
    char *data;
    struct data_block_list *next;
};

/* --- */

#endif
