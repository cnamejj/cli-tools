#ifndef ___CNAMEjj_SVG_IT_H__

#define ___CNAMEjj_SVG_IT_H__

#ifdef DEBUG_MALLOC
#include "bug_malloc.h"
#endif

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

struct context_info
{
    int ref;
    char *desc;
};

static struct context_info context_list[] =
{
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

#define IS_STDIN "-"
#define IS_STDOUT "-"
#define IS_BLANK " "

#define ST_NO_VALUE "-999999"

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
#define OP_IG_BAD_DATA 13
#define OP_DATA_DELIM  14
#define OP_CIRC_ALPHA  15
#define OP_DATA_ALPHA  16
#define OP_CFILL_ALPHA 17
#define OP_DFILL_ALPHA 18
#define OP_CIRC_RADIUS 19
#define OP_CIRC_LSIZE  20
#define OP_DATA_LSIZE  21
#define OP_KP_ALL_GOOD 22
#define OP_XMIN_VAL    23
#define OP_XMAX_VAL    24
#define OP_YMIN_VAL    25
#define OP_YMAX_VAL    26
#define OP_WIDTH       27
#define OP_HEIGHT      28
#define OP_DISP_WIDTH  29
#define OP_DISP_HEIGHT 30

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
#define FL_IG_BAD_DATA "ignore-bad-data"
#define FL_DATA_DELIM  "delim"
#define FL_CIRC_ALPHA  "circle-alpha"
#define FL_DATA_ALPHA  "data-alpha"
#define FL_CFILL_ALPHA "cfill-alpha"
#define FL_DFILL_ALPHA "dfill-alpha"
#define FL_CIRC_RADIUS "circle-radius"
#define FL_CIRC_LSIZE  "circle-line-size"
#define FL_DATA_LSIZE  "data-line-size"
#define FL_KP_ALL_GOOD "only-all-good"
#define FL_XMIN_VAL    "xmin"
#define FL_XMAX_VAL    "xmax"
#define FL_YMIN_VAL    "ymin"
#define FL_YMAX_VAL    "ymax"
#define FL_WIDTH       "width"
#define FL_HEIGHT      "height"
#define FL_DISP_WIDTH  "display-width"
#define FL_DISP_HEIGHT "display-height"

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
#define DEF_IG_BAD_DATA "0"
#define DEF_DATA_DELIM  IS_BLANK
#define DEF_CIRC_ALPHA  "0.4"
#define DEF_DATA_ALPHA  "0.6"
#define DEF_CFILL_ALPHA "0.3"
#define DEF_DFILL_ALPHA "0.0"
#define DEF_CIRC_RADIUS "22"
#define DEF_CIRC_LSIZE  "10"
#define DEF_DATA_LSIZE  ST_NO_VALUE
#define DEF_KP_ALL_GOOD "1"
#define DEF_XMIN_VAL    ST_NO_VALUE
#define DEF_XMAX_VAL    ST_NO_VALUE
#define DEF_YMIN_VAL    ST_NO_VALUE
#define DEF_YMAX_VAL    ST_NO_VALUE
#define DEF_WIDTH       ST_NO_VALUE
#define DEF_HEIGHT      ST_NO_VALUE
#define DEF_DISP_WIDTH  "100%"
#define DEF_DISP_HEIGHT "50%"

#define DATABUFFSIZE 8192

#define LABEL_META_FORMAT "%%.%df"

#define SC_XAX_GRIDS 7
#define SC_YAX_GRIDS 5
#define SC_TEXT_COLOR "#124488"
#define SC_AXIS_COLOR "#444444"
#define SC_CHART_COLOR "#FFFFFF"
#define SC_GRAPH_COLOR "#000000"
#define SC_DATA_FILL_COLOR "#FFFFFF"
#define SC_XGRID_COLOR "#2e2e2e"
#define SC_YGRID_COLOR "#2e2e2e"
#define SC_GRAPH_ALPHA 0.05
#define SC_CIRC_FILL_ALPHA 0.3
#define SC_CIRC_LINE_ALPHA 0.4
#define SC_DATA_FILL_ALPHA 0.0
#define SC_DATA_LINE_ALPHA 0.6

/* ---
 * Data series use the following default colors:
 *
 * S# CFILL   CLINE   DLINE
 * -- ------- ------- -------
 * 1. #088ea0 #e0da24 #744e18
 * 2. #659894 #3652a5 #4e1874
 * 3. #a98013 #84c061 #18744e
 * 4. #d02814 #9832a0 #233482
 * 5. matches #1
 * 6. matches #2
 * etc...
 *
 * S# - Data series number (the order they are added)
 * CF - Circle fill color
 * CL - Outline of circle
 * DL - Data line color
 *
 * Data fill is "transparent", color #ffffff w/ alpha 0.0, meaning no color under the
 * data lines.  The alpha values are: 0.4 for circle outlines, 0.3 for circle fills,
 * and 0.6 for data lines across all data series.
 */

struct data_series_visuals
{
    char *circle_fill, *circle_line, *data_line;
};

static struct data_series_visuals def_series_visuals[] = 
{
    {"#088ea0", "#e0da24", "#744e18"},
    {"#659894", "#3652a5", "#4e1874"},
    {"#a98013", "#84c061", "#18744e"}, 
    {"#d02814", "#9832a0", "#233482"},
};

#define SHOW_SYNTAX "\
Syntax is: %s <options>\n\
Options are:\n\
  <--title 'Chart Title'>\n\
  <--xtitle 'X-Axis Title'>\n\
  <--ytitle 'Y-Axis Title'>\n\
  <--xgrids ##>\n\
  <--ygrids ##>\n\
  <--out name-of-output-file>\n\
  <--debug ##>\n\
  <--help>\n\
  <--data name-of-input-file> | <--data ->\n\
  <--xcol ##>\n\
  <--ycol ##>\n\
  <--xdata> | <--no-xdata>\n\
  <--ydata> | <--no-ydata>\n\
  <--ignore-bad-data> | <--no-ignore-bad-data>\n\
  <--delim #>\n\
  <--circle-alpha #.##>\n\
  <--data-alpha #.##>\n\
  <--cfill-alpha #.##>\n\
  <--dfill-alpha #.##>\n\
  <--circle-radius ##>\n\
  <--circle-line-size ##>\n\
  <--data-line-size ##>\n\
  <--only-all-good> | <--no-only-all-good>\n\
  <--width ##>\n\
  <--height ##>\n\
\n\
If no output file is specified, the SVG document is written to STDOUT.  To read\n\
data from STDIN specify '--data -'.\n\
\n\
The default input field delimiter is ' ' and duplicates between words are ignored,\n\
similar to how AWK parses fields.  You can specific another strings to be used with\n\
'--delim', in which case repeated delimiters will not be ignored.\n\
\n\
The '--no-xdata' option directs the program to generate sequential numbers for the\n\
'x' data rather than parsing the input record.  And '--no-ydata' does the same thing\n\
for 'y'.  You can specific both but the resulting graph will be pretty pointless...\n\
\n\
Use '--no-only-good-data' if you want the code to retain a data point in an input\n\
record whether or not other fields in that records could be parsed.  Basically, treat\n\
each series (defined by an X/Y column pair) independently.\n\
\n\
The default values for the boolean flags are, --xdata, --ydata, --no-ignore-bad-data\n\
and --only-good-data.\n\
"  

/* --- */

struct parsed_options {
    int debug, help, xax_grids, yax_grids, *x_col_list, *y_col_list,
      x_data, y_data, ign_bad_data, nseries, circ_radius, circ_line_size,
      data_line_size, only_all_good, chart_width, chart_height;
    float circ_line_alpha, circ_fill_alpha, data_line_alpha,
      data_fill_alpha;
    double fix_xmin, fix_xmax, fix_ymin, fix_ymax;
    char *data_file, *out_file, *chart_title, *xax_title, *yax_title,
      *x_col_req, *y_col_req, *delim, *display_width, *display_height;
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

struct col_pair {
    int low, hi;
    struct col_pair *next;
};

/* --- */

char *context_desc( int context );

void bail_out( int rc, int err, int context, char *explain );

struct data_pair_list *load_data( struct parsed_options *popt );

void expand_series_col_req( struct parsed_options *popt );

int *parse_col_list_req( int *rc, int *ncols, char *req );

/* --- */

#endif
