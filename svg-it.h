#ifndef ___CNAMEjj_SVG_IT_H_

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

#define DS_ID_MARK '#'
#define DS_DESC_ID_PREF "S"
#define DS_DESC_ID_SUFF ":"
#define DS_DESC_XCOL_PREF " X/Col"
#define DS_DESC_YCOL_PREF " Y/Col"
#define NO_DESCRIPTION "No description"

#define NO_VALUE -999999
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
#define OP_RAW_DATA    31
#define OP_RAW_EOL     32
#define OP_LEGEND      33
#define OP_LSCALE      34
#define OP_DSNAME      35

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
#define FL_RAW_DATA    "raw-data"
#define FL_RAW_EOL     "raw-eol"
#define FL_LEGEND      "legend"
#define FL_LSCALE      "lscale"
#define FL_DSNAME      "series"

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
#define DEF_RAW_DATA    ""
#define DEF_RAW_EOL     ""
#define DEF_LEGEND      "0"
#define DEF_LSCALE      "20"
#define DEF_DSNAME      ""

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

#define CGI_RAW_EOL "\r\n"
#define CLI_RAW_EOL "\n"

#define SVG_RESPONSE_HEADER "\
Content-type: image/svg+xml\r\n\
\r\n\
"

#define SVG_TEXT_HEADER "\
<svg version=\"1.1\" baseProfile=\"full\" width=\"100%\" height=\"100%\"\n\
  viewbox=\"0 0 800 400\"\n\
  xmlns=\"http://www.w3.org/2000/svg\">\n\
\n\
<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\" />\n\
\n\
<g fill=\"#000000\" font-size=\"20\" fill-opacity=\"1.0\" style=\"font-family:Verdana, Arial, Helvetica, sans-serif; font-weight:100\">\n\
    <text style=\"font-size:100%\">\n\
"

#define SVG_TEXT_PREFIX "\
\n\
      <tspan x=\"1\" y=\"200\"\n\
        style=\"dominant-baseline:central; font-weight:100\" text-anchor=\"start\">\n\
\n\
"

#define SVG_TEXT_SUFFIX "\
</tspan>\n\
    </text>\n\
"

#define SVG_TEXT_TRAILER "\
</g>\n\
\n\
</svg>\n\
"

#define HTML_FORM_TEMPLATE "\
Content-type: text/html\r\n\
\r\n\
\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\
<html><head><title>SVG-It submit form</title>\n\
</head>\n\
<body bgcolor=\"#ffffff\">\n\
\n\
<br>Source for SVG-It is available in the <a href=\"https://github.com/cnamejj/cli-tools\">cli-tools</a>\
 repository on GitHub.\n\
<br>&nbsp;<br>\n\
\n\
<form action=\"http://<:SERVER:><:PORT:><:SCRIPT:>\" method=\"POST\">\n\
\n\
<table>\n\
  <tr><td colspan=\"2\"><b>Required Info/Settings:</b></td></tr>\n\
\
  <tr><td>Has X-Data?</td><td>Yes<input type=\"radio\" name=\"xdata\" value=\"yes\" checked>\n\
No<input type=\"radio\" name=\"xdata\" value=\"no\"></td></tr>\n\
  <tr><td>X Data Columns (comma delimited)</td><td><input type=\"text\" name=\"xcol\"></td></tr>\n\
\
  <tr><td>Has Y-Data?</td><td>Yes<input type=\"radio\" name=\"ydata\" value=\"yes\" checked>\n\
No<input type=\"radio\" name=\"ydata\" value=\"no\"></td></tr>\n\
  <tr><td>Y Data Columns (comma delimited)</td><td><input type=\"text\" name=\"ycol\"></td></tr>\n\
\
  <tr><td>Input Field Delimiter (default is space)</td><td><input type=\"text\" name=\"delim\"></td></tr>\n\
  <tr><td>Input Data:</td><td>&nbsp;</td></tr>\n\
  <tr><td colspan=\"2\"><textarea cols=\"100\" rows=\"12\" wrap=\"hard\" name=\"raw-data\"></textarea></tr>\n\
\
  <tr><td colspan=\"2\">&nbsp;</td></tr>\n\
  <tr><td colspan=\"2\"><b>Optional Bits:</b></td></tr>\n\
\
  <tr><td>Chart Title</td><td><input type=\"text\" name=\"title\"></td></tr>\n\
  <tr><td>X-Axis Title</td><td><input type=\"text\" name=\"xtitle\"></td></tr>\n\
  <tr><td>Y-Axis Title</td><td><input type=\"text\" name=\"ytitle\"></td></tr>\n\
\
  <tr><td># of X-Axis Grids</td><td><input type=\"text\" name=\"xgrids\" value=\"4\"></td></tr>\n\
  <tr><td># of Y-Axis Grids</td><td><input type=\"text\" name=\"ygrids\" value=\"4\"></td></tr>\n\
\
  <tr><td>Ignore Bad Data?</td><td>Yes<input type=\"radio\" name=\"ignore-bad-data\" value=\"yes\" checked>\n\
No<input type=\"radio\" name=\"ydata\" value=\"no\"></td></tr>\n\
  <tr><td>Use Partially Good Input Records?</td><td>Yes<input type=\"radio\" name=\"only-all-good\" value=\"no\" checked>\n\
No<input type=\"radio\" name=\"only-all-good\" value=\"yes\"></td></tr>\n\
\
  <tr><td>Data Point Alpha Value (0.0-1.0)</td><td><input type=\"text\" name=\"circle-alpha\" value=\"0.6\"></td></tr>\n\
  <tr><td>Data Line Alpha Value (0.0-1.0)</td><td><input type=\"text\" name=\"data-alpha\" value=\"0.6\"></td></tr>\n\
  <tr><td>Data Point Fill Alpha Value (0.0-1.0)</td><td><input type=\"text\" name=\"cfill-alpha\" value=\"0.3\"></td></tr>\n\
  <tr><td>Data Point Radius (automatic if -999999)</td><td><input type=\"text\" name=\"circle-radius\" value=\"-999999\"></td></tr>\n\
  <tr><td>Data Point Outline Size (automatic if -999999)</td><td><input type=\"text\" name=\"circle-line-size\" value=\"-999999\"></td></tr>\n\
  <tr><td>Data Line Width (automatic if -999999)</td><td><input type=\"text\" name=\"data-line-size\" value=\"-999999\"></td></tr>\n\
\
  <tr><td>Forced Minimum X Value (autodetect if -999999)</td><td><input type=\"text\" name=\"xmin\" value=\"-999999\"></td></tr>\n\
  <tr><td>Forced Maximum X Value (autodetect if -999999)</td><td><input type=\"text\" name=\"xmax\" value=\"-999999\"></td></tr>\n\
  <tr><td>Forced Minimum Y Value (autodetect if -999999)</td><td><input type=\"text\" name=\"ymin\" value=\"-999999\"></td></tr>\n\
  <tr><td>Forced Maximum Y Value (autodetect if -999999)</td><td><input type=\"text\" name=\"ymax\" value=\"-999999\"></td></tr>\n\
  <tr><td>Chart Width</td><td><input type=\"text\" name=\"width\" value=\"1000\"></td></tr>\n\
  <tr><td>Chart Height</td><td><input type=\"text\" name=\"height\" value=\"400\"></td></tr>\n\
  <tr><td>Chart Display Width (window percentage)</td><td><input type=\"text\" name=\"display-width\" value=\"100%\"></td></tr>\n\
  <tr><td>Chart Display Height (window percentage)</td><td><input type=\"text\" name=\"display-height\" value=\"100%\"></td></tr>\n\
\
  <tr><td>Include legend?</td><td>Yes<input type=\"radio\" name=\"legend\" value=\"1\">\n\
No<input type=\"radio\" name=\"legend\" value=\"0\" checked></td></tr>\n\
  <tr><td>Legend Size (as % of chart width, default is 20)</td><td><input type=\"text\" name=\"lscale\"></td></tr>\n\
\
  <tr><td>Data Series #1 Name (defaults to column number info)</td><td><input type=\"text\" name=\"series\"></td></tr>\n\
  <tr><td>Data Series #2 Name</td><td><input type=\"text\" name=\"series\"></td></tr>\n\
  <tr><td>Data Series #3 Name</td><td><input type=\"text\" name=\"series\"></td></tr>\n\
  <tr><td>Data Series #4 Name</td><td><input type=\"text\" name=\"series\"></td></tr>\n\
  <tr><td>Data Series #5 Name</td><td><input type=\"text\" name=\"series\"></td></tr>\n\
  <tr><td>Data Series #6 Name</td><td><input type=\"text\" name=\"series\"></td></tr>\n\
\
  <tr><td>Debug? (view SVG source for info)</td><td>Yes<input type=\"radio\" name=\"debug\" value=\"1\">\n\
No<input type=\"radio\" name=\"debug\" value=\"0\" checked></td></tr>\n\
\
  <tr><td>&nbsp;</td><td>&nbsp;</td></tr>\n\
\
  <tr><td>&nbsp;</td><td><input type=\"submit\" value=\"SVG-It\"></td></tr>\n\
</table>\n\
\n\
</form>\n\
\n\
</body>\n\
</html>\n\
"

/* ---
 * Data series use the following default colors:
 *
 * S# CFILL   CLINE   DLINE
 * -- ------- ------- -------
 * 1. #088ea0 #e0da24 #744e18
 * 2. #659894 #3652a5 #4e1874
 * 3. #d02814 #9832a0 #233482
 * 4. #75851f #a87323 #a42334
 * 5. #a98013 #84c061 #18744e
 * 6. #ee8e8e #1f0808 #573514
 * 7. matches #1
 * 8. matches #2
 * 9. matches #3
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
    {"#d02814", "#9832a0", "#233482"},
    {"#75851f", "#a87323", "#a42334"},
    {"#a98013", "#84c061", "#18744e"}, 
    {"#ee8e8e", "#1f0808", "#573514"},
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
  <--display-width ##>\n\
  <--display-height ##>\n\
  <--legend> | <--no-legend>\n\
  <--lscale ##>\n\
  <--series 'data series name#1'> <--series 'data series name#2'> etc...\n\
\n\
If no output file is specified, the SVG document is written to STDOUT.  To\n\
read data from STDIN specify '--data -'.\n\
\n\
The default input field delimiter is ' ' and duplicates between words are\n\
ignored, similar to how AWK parses fields.  You can specific another strings\n\
to be used with '--delim', in which case repeated delimiters will not be\n\
ignored.\n\
\n\
The '--no-xdata' option directs the program to generate sequential numbers for\n\
the 'x' data rather than parsing the input record.  And '--no-ydata' does the\n\
same thing for 'y'.  You can specific both but the resulting graph will be\n\
pretty pointless...\n\
\n\
Use '--no-only-good-data' if you want the code to retain a data point in an\n\
input record whether or not other fields in that records could be parsed.\n\
Basically, treat each series (defined by an X/Y column pair) independently.\n\
\n\
The default values for the boolean flags are, --xdata, --ydata,\n\
--no-ignore-bad-data and --only-good-data.\n\
\n\
Flags --width and --height specify the size of chart in pixels, used in the\n\
'viewbox' parameter of the SVG document.  Flags --display-width and\n\
--display-height indicate how browsers should render the chart relative to the\n\
size of the page they are on and can be given as a percentage (the %% should\n\
should be included). The defaults are 3000x900 for width/height and 49%%/25%%\n\
for display-width/display-height.\n\
\n\
By default no legend is included.  Use '--legend' to request one on the right\n\
side of the chart.  The width of the legend, as a percentage of the chart\n\
width, can be specified with '--lscale' and defaults to 20.\n\
\n\
By default, the descriptions used in the legend for each data series will be\n\
the X/Y column numbers in the input dataset.  You can substitute alternate\n\
descriptions with the '--series' flag.  If the description starts with a pound\n\
sign '#' and number, it will be assigned to the corresponding data series\n\
number.  Otherwise the '--series' flags will be assigned to the data series in\n\
the order they are given on the command line.\n\
"


/* --- */

struct parsed_options {
    int debug, help, xax_grids, yax_grids, *x_col_list, *y_col_list,
      x_data, y_data, ign_bad_data, nseries, circ_radius, circ_line_size,
      data_line_size, only_all_good, chart_width, chart_height, html_out,
      has_legend, legend_scale;
    float circ_line_alpha, circ_fill_alpha, data_line_alpha,
      data_fill_alpha;
    double fix_xmin, fix_xmax, fix_ymin, fix_ymax;
    char *data_file, *out_file, *chart_title, *xax_title, *yax_title,
      *x_col_req, *y_col_req, *delim, *display_width, *display_height,
      *raw_data, *raw_eol;
    struct value_chain *dsname;
};

struct data_pair_list {
    int cases, named;
    double *xval, *yval;
    char *desc;
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

void bail_out( int rc, int err, int html_out, int context, char *explain );

struct data_pair_list *load_data( struct parsed_options *popt );

void expand_series_col_req( struct parsed_options *popt );

int *parse_col_list_req( int *rc, int *ncols, char *req );

void show_form_and_exit();

char *comm_op( int html_out );

char *comm_cl( int html_out );

void free_loaded_data( struct data_pair_list *data, int nseries );

void free_command_flags( struct option_set *ops, int nflags );

void clear_parsed_options( struct parsed_options *popt );

char *gen_data_series_desc( int *rc, int dsid, int xin, int yin );

/* --- */

#endif
