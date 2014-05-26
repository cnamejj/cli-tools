#include <openssl/ssl.h>

#include "http-fetch.h"
#include "err_ref.h"

/* stub routine for now... */

long bio_ssl_callback(BIO *bn, int flags, const char *buff, int blen, long ignore, long ret)

{
    int rc;
    struct plan_data *plan;
    struct fetch_status *fetch;
    struct output_options *out;
    struct display_settings *disp;

    /* --- */

    plan = register_current_plan( 0);
    fetch = plan->status;
    out = plan->out;
    disp = plan->disp;

    if(flags == (BIO_CB_RETURN | BIO_CB_READ) )
    {
        if( out->debug_level >= DEBUG_HIGH1) fprintf( out->info_out,
          "%sSSL read from socket, bytes=%ld\n", disp->line_pref, ret);

        if( ret > 0)
        {
            rc = capture_checkpoint( fetch, EVENT_SSL_NET_READ);
            if( rc == RC_NORMAL) rc = add_datalen_block( fetch->lastcheck, ret);
	}
    }
    else if(flags == (BIO_CB_RETURN | BIO_CB_WRITE) )
    {
        if( out->debug_level >= DEBUG_HIGH1) fprintf( out->info_out,
          "%sSSL write to socket, bytes=%ld\n", disp->line_pref, ret);

        if( ret > 0)
        {
            rc = capture_checkpoint( fetch, EVENT_SSL_NET_WRITE);
            if( rc == RC_NORMAL) rc = add_datalen_block( fetch->lastcheck, ret);
	}
    }

    return( ret);
}
