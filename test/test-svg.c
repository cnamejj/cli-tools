#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "../cli-sub.h"
#include "../svg-tools.h"
#include "../err_ref.h"

#ifdef INLINE_EXPLODE_STRING
struct string_parts {
   int np;
   char **list;
};

struct string_parts *explode_string( int *rc, char *string, char *delim);
#endif

#define CASES 25
#define SCOPE 200

#define LEAVE(MSG) \
{ \
    fprintf( stderr, "Err: %s\n", MSG); \
    exit( 1); \
}

int main(int narg, char **opts)

{
    int rc = 0, seed, recs, datasize, inp, seq, off, digits;
    float *xval = 0, *yval = 0, xinp, yinp;
/*    char *yax = 0, *xax = 0, *xgrid = 0, *ygrid = 0, *circ = 0, *pstart = 0, *lines = 0; */
    double span;
    char *all = 0, *data = 0, *databuff = 0, *df = 0;
    struct svg_model *chart = 0;
    struct stat st_buff;
    struct string_parts *exp = 0;
    time_t now;

    if( narg > 1)
    {
        data = opts[ 1];

        rc = stat( data, &st_buff);
        if( rc) LEAVE( "Call to stat() for input file failed.")

        datasize = st_buff.st_size;
        if( datasize < 1) LEAVE( "Data file is empty!")

        databuff = (char *) malloc( datasize + 1);
        if( !databuff) LEAVE( "Call to malloc() failed")

        inp = open( data, O_RDONLY);
        if( inp == -1) LEAVE( "Can't open data file")

        rc = read( inp, databuff, datasize);
        if( rc != datasize) LEAVE( "Read didn't yield the number of bytes expected")

        *(databuff + datasize) = '\0';

        exp = explode_string( &rc, databuff, "\n");
        if( rc != RC_NORMAL) LEAVE( "Call to explode_string() failed")

        recs = exp->np;
        if( recs < 3) LEAVE( "Less then 3 records in data file")

        xval = (float *) malloc( recs * (sizeof *xval));
        yval = (float *) malloc( recs * (sizeof *yval));

        seq = -1;
        for( off = 0; off < recs; off++)
        {
            rc = sscanf( exp->list[ off], "%f %f", &xinp, &yinp);
            if( rc == 2)
            {
                seq++;
                xval[ seq] = xinp;
                yval[ seq] = yinp;
                printf( "<!-- Data: %f %f -->\n", xval[ seq], yval[ seq]);
	    }
	}

        if( seq < 3) LEAVE( "Less than 3 records with x/y values")
        recs = seq;
    }
    else
    {
        (void) time( &now);
        seed = (now & 0xffff) * getpid();
        srand( seed);

        xval = (float *) malloc( CASES * (sizeof *xval));
        yval = (float *) malloc( CASES * (sizeof *yval));

        for( off = 0; off < CASES; off++)
        {
            *(xval + off) = (float) (off + 1);
            *(yval + off) = (float) (rand() % SCOPE);
/*            printf( "<!-- Data: %d. %.0f -->\n", off, *(yval + off)); */
        }

        recs = CASES;
    }

    chart = svg_make_chart();
    if( !chart) LEAVE( "Call to svg_make_chart() failed")

    rc = svg_add_float_data( chart, recs, xval, yval);
    if( rc != RC_NORMAL) LEAVE( "Call to svg_add_float_data() failed")

/*
    rc = svg_finalize_model( chart);
    if( rc != RC_NORMAL) LEAVE( "Call to svg_finalize_model() failed")

    yax = svg_make_yax_labels( &rc, chart);
    if( rc != RC_NORMAL) LEAVE( "Call to svg_make_yax_labels() failed")
    if( !yax) yax = "N/A";
    fprintf( stdout, "<!-- yax:\n%s -->\n", yax);

    xax = svg_make_xax_labels( &rc, chart);
    if( rc != RC_NORMAL) LEAVE( "Call to svg_make_xax_labels() failed")
    if( !xax) xax = "N/A";
    fprintf( stdout, "<!-- xax:\n%s -->\n", xax);

    ygrid = svg_make_yax_grid( &rc, chart);
    if( rc != RC_NORMAL) LEAVE( "Call to svg_make_yax_grid() failed")
    if( !ygrid) ygrid = "N/A";
    fprintf( stdout, "<!-- ygrid:\n%s -->\n", ygrid);

    xgrid = svg_make_xax_grid( &rc, chart);
    if( rc != RC_NORMAL) LEAVE( "Call to svg_make_xax_grid() failed")
    if( !xgrid) xgrid = "N/A";
    fprintf( stdout, "<!-- xgrid:\n%s -->\n", xgrid);

    circ = svg_make_data_points( &rc, chart);
    if( rc != RC_NORMAL) LEAVE( "Call to svg_make_data_points() failed")
    if( !circ) circ = "N/A";
    fprintf( stdout, "<!-- circ:\n%s -->\n", circ);

    pstart = svg_make_path_start( &rc, chart);
    if( rc != RC_NORMAL) LEAVE( "Call to svg_make_path_start() failed")
    if( !pstart) pstart = "N/A";
    fprintf( stdout, "<!-- pstart:\n%s -->\n", pstart);

    lines = svg_make_data_lines( &rc, chart);
    if( rc != RC_NORMAL) LEAVE( "Call to svg_make_data_lines() failed")
    if( !lines) lines = "N/A";
    fprintf( stdout, "<!-- lines:\n%s -->\n", lines);
 */

    rc = svg_set_chart_title( chart, "Random data");
    rc = svg_set_xax_title( chart, "x-axis data");
    rc = svg_set_yax_title( chart, "y-axis data");

    rc = svg_set_yax_num_grids( chart, 5 );
    rc = svg_set_xax_num_grids( chart, 10 );

/*    rc = svg_set_text_size( chart, ???); */

    span = (chart->ymax - chart->ymin) / 5;
    if( span >= 1.0 ) rc = svg_set_yax_disp( chart, "%.0f");
    else
    {
        digits = (int) (1 - log10(span));
        df = string_from_int( &rc, digits, "%%.%df");
        rc = svg_set_yax_disp( chart, df);
        free( df);
/*        printf( "<!-- auto-digits: y: rc:%d sp:%f dig:%d df(%s) -->\n", rc, span, digits, df); */
      }

    span = (chart->xmax - chart->xmin) / 10;
    if( span >= 1.0) rc = svg_set_xax_disp( chart, "%.0f");
    else
    {
        digits = (int) (1 - log10(span));
        df = string_from_int( &rc, digits,"%%.%df");
        rc = svg_set_xax_disp( chart, df);
        free( df);
/*        printf( "<!-- auto-digits: x: rc:%d sp:%f dig:%d df(%s) -->\n", rc, span, digits, df); */
    }

    rc = svg_set_screen_width( chart, "49%");
    rc = svg_set_screen_height( chart, "20%");

/*
    rc = svg_set_reserve_height( chart, ???);
    rc = svg_set_reserve_width( chart, ???);
    rc = svg_set_shift_height( chart, ???);
    rc = svg_set_shift_width( chart, ???);
    rc = svg_set_shift_bottom( chart, ???);
 */

/*    rc = svg_set_data_line_size( chart, 8); */
    rc = svg_set_circ_line_size( chart, 10);
    rc = svg_set_circ_radius( chart, 22);

/*
    rc = svg_set_x_gridline_size( chart, 6);
    rc = svg_set_y_gridline_size( chart, 2);
    rc = svg_set_axis_size( chart, 4);
 */

    rc = svg_set_text_color( chart, "#124488");
    rc = svg_set_axis_color( chart, "#444444");
    rc = svg_set_chart_color( chart, "#FFFFFF");
    rc = svg_set_graph_color( chart, "#000000");
    rc = svg_set_circ_fill_color( chart, "#088ea0");
    rc = svg_set_circ_line_color( chart, "#e0da24");
    rc = svg_set_data_fill_color( chart, "#FFFFFF");
    rc = svg_set_data_line_color( chart, "#744e18");
    rc = svg_set_x_gridline_color( chart, "#2e2e2e");
    rc = svg_set_y_gridline_color( chart, "#2e2e2e");

/*    rc = svg_set_axis_alpha( chart, 0.02); */
    rc = svg_set_graph_alpha( chart, 0.05);
/*    rc = svg_set_chart_alpha( chart, 0.1); */
/*    rc = svg_set_text_alpha( chart, 0.1); */
    rc = svg_set_circ_fill_alpha( chart, 0.3);
    rc = svg_set_circ_line_alpha( chart, 0.4);
    rc = svg_set_data_fill_alpha( chart, 0.0);
    rc = svg_set_data_line_alpha( chart, 0.6);
/*    rc = svg_set_x_gridline_alpha( chart, 0.1); */
/*    rc = svg_set_y_gridline_alpha( chart, 0.1); */

/*    rc = svg_set_xmin( chart, 10.0 ); */
/*    rc = svg_set_xmax( chart, 30.0 ); */
/*    rc = svg_set_ymin( chart, 0.0 ); */
/*    rc = svg_set_ymax( chart, 100.0 ); */

    all = svg_render( &rc, chart);
    if( rc != RC_NORMAL) LEAVE( "Call to svg_render() failed")
    if( !all) all = "N/A";
    fprintf( stdout, "%s\n", all);

    exit(0);
}

#ifdef INLINE_EXPLODE_STRING

/* --- */

struct string_parts *explode_string( int *rc, char *string, char *delim)

{
   int dlen, count, ii, err = ERR_BAD_PARMS;
   char *st, *buff = 0;
   struct string_parts *rez = 0;

   if( string && delim) if( *string && *delim) err = RC_NORMAL;

   if( err == RC_NORMAL)
   {
      buff = strdup( string);
      if( !buff) err = ERR_MALLOC_FAILED;
      else
      {
         rez = (struct string_parts *) malloc( sizeof *rez);
         if( !rez) err = ERR_MALLOC_FAILED;
      }
   }

   if( err == RC_NORMAL)
   {
      rez->np = 0;
      rez->list = 0;

      dlen = strlen( delim);
      count = 1;

      for( st = buff; *st; )
      {
         if( strncmp( st, delim, dlen)) st++;
         else
         {
            count++;
            *st = '\0';
            st += dlen;
         }
      }

      rez->list = (char **) malloc( count * (sizeof *(rez->list)));
      if( !rez->list) err = ERR_MALLOC_FAILED;
      else rez->np = count;
   }

   if( err == RC_NORMAL)
   {
      for( ii = 0; ii < count; ii++)
        rez->list[ ii] = 0;

      for( ii = 0, st = buff; ii < count && err == RC_NORMAL; ii++)
      {
         rez->list[ ii] = strdup( st);
         if( !rez->list[ ii]) err = ERR_MALLOC_FAILED;
         else st += strlen( st) + dlen;;
      }

      if( buff) free( buff);
   }

   if( rc) *rc = err;

   return rez;
}

#endif
