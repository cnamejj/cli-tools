/*
 * #define SHOW_DEBUG_STATS_CALCS
 * #define SHOW_GORY_XFRATE_CALC_DETAILS
 */

#include "http-fetch.h"
#include "cli-sub.h"
#include "err_ref.h"

/*
struct stat_work {
    int *packsize;
    long packsize_sum;
    float *xfrate, *readlag;
    float packsize_max, packsize_mean, xfrate_sum,
      xfrate_max, xfrate_mean, readlag_sum,
      readlag_max, readlag_mean;
};
 */

/* --- */

void calc_xfrates( int want_event, struct fetch_status *status, struct stat_work *swork)

{
    int is_last_data = 0, off, *packsize, packets;
    long pack_sum = 0;
    float *readlag, *xfrate;
    float elap, rate, lag_sum = 0.0, lag_max = 0.0, lag_mean = 0.0,
      xfr_sum = 0.0, xfr_max = 0.0, xfr_mean = 0.0,
      pack_max = 0.0, pack_mean = 0.0;
    struct ckpt_chain *walk, *prevpack;
#ifdef SHOW_GORY_XFRATE_CALC_DETAILS
    int dbgfirst = 0, initial_size = 0;
    float r1, r2;
    char m1, m2;
#endif

    walk = status->checkpoint;
    packsize = swork->packsize;
    readlag = swork->readlag;
    xfrate = swork->xfrate;
    packets = swork->samples;

    for( off = 0; walk; walk = walk->next) if( walk->event == want_event && walk->detail)
    {
        is_last_data = (off + 1) >= packets;

        /* The last packet is usually partial, so it should be ignored */
        if( !is_last_data)
        {
#ifdef SHOW_GORY_XFRATE_CALC_DETAILS
            if( off) initial_size = walk->detail->len;
#endif
            pack_sum += walk->detail->len;
            *(packsize + off) = walk->detail->len;
            if( (float) walk->detail->len > pack_max) pack_max = (float) walk->detail->len;
        }

        /* Lag is calculate from the last read, so the first packet has none */
        if( off)
        {
            elap = calc_time_difference( &prevpack->clock, &walk->clock, status->clock_res);
            lag_sum += elap;
            *(readlag + off - 1) = elap;
            if( elap > lag_max) lag_max = elap;
        }

        /* Per packet transfer rates only make sense for the 2nd to next to last packets */
        if( off && !is_last_data)
        {
            if( !walk->detail->len || elap == 0.0) rate = 0.0;
            else rate = ((float) walk->detail->len) / elap;
            xfr_sum += rate;
            *(xfrate + off - 1) = rate;
            if( rate > xfr_max) xfr_max = rate;

#ifdef SHOW_GORY_XFRATE_CALC_DETAILS
            if( !dbgfirst++)
            {
                printf( "dbg:: Xf_Sum Seq.  Elapsed  Size      Xfer   Xfer-Rate-Sum  SzSum   Read-Lag-Sum  Run-Xfer\n");
                printf( "dbg:: Xf_Sum ---. -------- ----- --------- --------------- ------ -------------- ---------\n");
            }

            r1 = get_scaled_number( &m1, rate);
            r2 = get_scaled_number( &m2, (packsize_sum - initial_size) / readlag_sum);
            printf( "dbg:: Xf_Sum %3d. %8.6f %5d %8.3f%c %15.4f %6ld %14.6f %8.3f%c\n", off, elap, walk->detail->len, 
              r1, m1, xfrate_sum, packsize_sum - initial_size, readlag_sum, r2, m2);
#endif
	    }

        prevpack = walk;
        off++;
    }

    if( !packets)
    {
        lag_mean = pack_mean = xfr_mean = 0.0;
    }
    else if( packets == 1)
    {
        lag_mean = lag_sum;
        pack_mean = (float) pack_sum;
        xfr_mean = 0.0;
    }
    else
    {
        packets--;
        lag_mean = lag_sum / packets;
        pack_mean = ((float) pack_sum) / packets;
        if( packets == 1) xfr_mean = xfr_sum;
        else xfr_mean = xfr_sum / (packets - 1);
    }

    swork->samples = packets;
    swork->packsize_sum = pack_sum;
    swork->packsize_max = pack_max;
    swork->packsize_mean = pack_mean;
    swork->xfrate_sum = xfr_sum;
    swork->xfrate_max = xfr_max;
    swork->xfrate_mean = xfr_mean;
    swork->readlag_sum = lag_sum;
    swork->readlag_max = lag_max;
    swork->readlag_mean = lag_mean;

    return;
}
