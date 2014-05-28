/*
 * #define SHOW_DEBUG_STATS_CALCS
 * #define SHOW_GORY_XFRATE_CALC_DETAILS
 */

#include <math.h>

#include "http-fetch.h"
#include "err_ref.h"

/* --- */

void calc_standard_moments( struct fetch_status *status, struct stat_work *swork)

{
    int off, npack, *packsize;
    float *readlag, *xfrate, mean_diff, adj, adj2, samp_sz, samp_div, variance,
      packsize_max, readlag_max, xfrate_max,
      packsize_mean, readlag_mean, xfrate_mean,
      packsize_msd_norm = 0.0, readlag_msd_norm = 0.0, xfrate_msd_norm = 0.0,
      packsize_mth_norm = 0.0, readlag_mth_norm = 0.0, xfrate_mth_norm = 0.0,
      packsize_mfo_norm = 0.0, readlag_mfo_norm = 0.0, xfrate_mfo_norm = 0.0,
      packsize_norm_stdev = 0.0, readlag_norm_stdev = 0.0, xfrate_norm_stdev = 0.0,
      packsize_norm_skew = 0.0, readlag_norm_skew = 0.0, xfrate_norm_skew = 0.0,
      packsize_norm_kurt = 0.0, readlag_norm_kurt = 0.0, xfrate_norm_kurt = 0.0;
#ifdef SHOW_DEBUG_STATS_CALCS
    int packsize_sum;
    float readlag_sum, xfrate_sum;
#endif

    npack = swork->samples;
    packsize = swork->packsize;
    readlag = swork->readlag;
    xfrate = swork->xfrate;

    packsize_max = swork->packsize_max;
    packsize_mean = swork->packsize_mean;
    readlag_max = swork->readlag_max;
    readlag_mean = swork->readlag_mean;
    xfrate_max = swork->xfrate_max;
    xfrate_mean = swork->xfrate_mean;

#ifdef SHOW_DEBUG_STATS_CALCS
    packsize_sum = swork->packsize_sum;
    readlag_sum = swork->readlag_sum;
    xfrate_sum = swork->xfrate_sum;
#endif

#ifdef SHOW_GORY_XFRATE_CALC_DETAILS
    printf( "dbg:: - - -\n");
#endif

    for( off = 0; off < npack; off++)
    {
        if( packsize_max)
        {
            mean_diff = (((float) *(packsize + off)) - packsize_mean) / packsize_max;
            adj = mean_diff * mean_diff;
            packsize_msd_norm += adj;
            adj *= mean_diff;
            packsize_mth_norm += adj;
            adj *= mean_diff;
            packsize_mfo_norm += adj;
        }

        if( readlag_max)
        {
            mean_diff = (*(readlag + off) - readlag_mean) / readlag_max;
            adj = mean_diff * mean_diff;
            readlag_msd_norm += adj;
            adj *= mean_diff;
            readlag_mth_norm += adj;
            adj *= mean_diff;
            readlag_mfo_norm += adj;
	}

        if( off < npack - 1 && xfrate_max)
        {
            mean_diff = (*(xfrate + off) - xfrate_mean) / xfrate_max;
            adj = mean_diff * mean_diff;
            xfrate_msd_norm += adj;
            adj *= mean_diff;
            xfrate_mth_norm += adj;
            adj *= mean_diff;
            xfrate_mfo_norm += adj;
	}
    }

#ifdef SHOW_DEBUG_STATS_CALCS
    printf( "dbg:: - - - %d\n", npack);
    printf( "dbg:: What:        Sum       Mean        Max MSqDiffSum MThDiffSum MFoDiffSum\n");
    printf( "dbg:: ----- ---------- ---------- ---------- ---------- ---------- ----------\n");
    printf( "dbg:: Lag:: %10.3e %10.3e %10.3e %10.3e %10.3e %10.3e\n", readlag_sum,
      readlag_mean, readlag_max, readlag_msd_norm, readlag_mth_norm, readlag_mfo_norm);
    printf( "dbg:: Pack: %10d %10.3e %10.3e %10.3e %10.3e %10.3e\n", packsize_sum,
      packsize_mean, packsize_max, packsize_msd_norm, packsize_mth_norm, packsize_mfo_norm);
    printf( "dbg:: XRat: %10.3e %10.3e %10.3e %10.3e %10.3e %10.3e\n", xfrate_sum,
      xfrate_mean, xfrate_max, xfrate_msd_norm, xfrate_mth_norm, xfrate_mfo_norm);
#endif

    if( npack >= 2)
    {
        /* When calculating the variance/stdev from a sample popultion you divide by N-1 */
        adj = (float) npack - 1;
        packsize_norm_stdev = sqrt( packsize_msd_norm / adj);
        readlag_norm_stdev = sqrt( readlag_msd_norm / adj);
        if( npack == 2) xfrate_norm_stdev = 0.0;
        else xfrate_norm_stdev = sqrt( xfrate_msd_norm / (adj - 1.0));


        if( npack >= 3)
        {
            adj = (float) (npack) / (float) ((npack - 1) * (npack - 2));

            adj2 = pow( packsize_norm_stdev, 3.0);
            if( !adj2) packsize_norm_skew = 0.0;
            else packsize_norm_skew = adj * (packsize_mth_norm / adj2);

            adj2 = pow( readlag_norm_stdev, 3.0);
            if( !adj2) readlag_norm_skew = 0.0;
            else readlag_norm_skew = adj * (readlag_mth_norm / adj2);

            if( npack >= 4)
            {
                adj = (float) (npack - 1) / (float) ((npack - 2) * (npack - 3));

                adj2 = pow( xfrate_norm_stdev, 3.0);
                if( !adj2) xfrate_norm_skew = 0.0;
                else xfrate_norm_skew = adj * (xfrate_mth_norm / adj2);
            }

            if( npack >= 4)
            {
                samp_sz = (float) npack;
                samp_div = samp_sz - 1.0;
                adj = (samp_sz * (samp_sz + 1)) / ( (samp_div * (samp_div - 1) * (samp_div - 2) ) );
                adj2 = 3 * ( (samp_div * samp_div) / ( (samp_div - 1) * (samp_div - 2) ) );

                variance = packsize_msd_norm / samp_div;
                if( !variance) packsize_norm_kurt = 0.0;
                else packsize_norm_kurt = adj * ( packsize_mfo_norm / (variance * variance) ) - adj2;
#ifdef SHOW_DEBUG_STATS_CALCS
                printf( "dbg:: Stat: What: AdjFactor1 MFoDiffSum   Variance AdjFactor2       Kurt\n");
                printf( "dbg:: ----- ----- ---------- ---------- ---------- ---------- ----------\n");
                printf( "dbg:: Kurt: Pack: %10.3e %10.3e %10.3e %10.3e %10.3e\n", adj, packsize_mfo_norm,
                  variance, adj2, packsize_norm_kurt);
#endif

                variance = readlag_msd_norm / samp_div;
                if( !variance) readlag_norm_kurt = 0.0;
                else readlag_norm_kurt = adj * ( readlag_mfo_norm / (variance * variance) ) - adj2;
#ifdef SHOW_DEBUG_STATS_CALCS
                printf( "dbg:: Kurt: Lag:: %10.3e %10.3e %10.3e %10.3e %10.3e\n", adj, readlag_mfo_norm,
                  variance, adj2, readlag_norm_kurt);
#endif

		if( npack >= 5)
                {
                    samp_sz = (float) (npack - 1);
                    samp_div = samp_sz - 1.0;
                    adj = (samp_sz * (samp_sz + 1)) / ( (samp_div * (samp_div - 1) * (samp_div - 2) ) );
                    adj2 = 3 * ( (samp_div * samp_div) / ( (samp_div - 1) * (samp_div - 2) ) );

                    variance = xfrate_msd_norm / samp_div;
                    if( !variance) xfrate_norm_kurt = 0.0;
                    else xfrate_norm_kurt = adj * ( xfrate_mfo_norm / (variance * variance) ) - adj2;
#ifdef SHOW_DEBUG_STATS_CALCS
                    printf( "dbg:: Kurt: XRat: %10.3e %10.3e %10.3e %10.3e %10.3e\n", adj, xfrate_mfo_norm,
                      variance, adj2, xfrate_norm_kurt);
#endif
                }
            }

#ifdef SHOW_DEBUG_STATS_CALCS
            printf( "dbg:: Stat: What: AdjFactor1 MThDiffSum     StdDev   StDev**3       Skew\n");
            printf( "dbg:: ----- ----- ---------- ---------- ---------- ---------- ----------\n");
            printf( "dbg:: Skew: Lag:: %10.3e %10.3e %10.3e %10.3e %10.3e\n",
              (float) (npack) / (float) ((npack - 1) * (npack - 2)), readlag_mth_norm,
            readlag_norm_stdev, pow( readlag_norm_stdev, 3.0), readlag_norm_skew);
            printf( "dbg:: Skew: Pack: %10.3e %10.3e %10.3e %10.3e %10.3e\n",
              (float) (npack) / (float) ((npack - 1) * (npack - 2)), packsize_mth_norm,
              packsize_norm_stdev, pow( packsize_norm_stdev, 3.0), packsize_norm_skew);
            printf( "dbg:: Skew: XRat: %10.3e %10.3e %10.3e %10.3e %10.3e\n", adj, xfrate_mth_norm,
              xfrate_norm_stdev, pow( xfrate_norm_stdev, 3.0), xfrate_norm_skew);
#endif
	}
    }

    swork->packsize_norm_stdev = packsize_norm_stdev;
    swork->packsize_norm_skew = packsize_norm_skew;
    swork->packsize_norm_kurt = packsize_norm_kurt;

    swork->readlag_norm_stdev = readlag_norm_stdev;
    swork->readlag_norm_skew = readlag_norm_skew;
    swork->readlag_norm_kurt = readlag_norm_kurt;

    swork->xfrate_norm_stdev = xfrate_norm_stdev;
    swork->xfrate_norm_skew = xfrate_norm_skew;
    swork->xfrate_norm_kurt =xfrate_norm_kurt;

    return;
}
