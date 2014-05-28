#include <openssl/ssl.h>

#include "cli-sub.h"
#include "http-fetch.h"
#include "err_ref.h"

/* --- */

void setup_ssl_env( int *rc, struct plan_data *plan)

{
    unsigned long hold_err;
    struct target_info *req = 0;
    struct fetch_status *fetch;

    if( *rc == RC_NORMAL)
    {
        fetch = plan->status;
        if( plan->redirect) if( plan->redirect->conn_url) if( *plan->redirect->conn_url) req = plan->redirect;
        if( !req) req = plan->target;

        if( !req || !fetch) *rc = ERR_UNSUPPORTED;
        else if( req->use_ssl && !fetch->ssl_context)
        {
            fetch->ssl_context = init_ssl_context( verify_ssl_callback);
            if( !fetch->ssl_context)
            {
                *rc = ERR_SSL_INIT_FAILED;
                hold_err = ERR_peek_error();
                if( hold_err)
                {
                    fetch->end_errno = hold_err;
                    (void) stash_ssl_err_info( fetch, hold_err);
		}
                else
                {
                    fetch->end_errno = errno;
                    fetch->err_msg = strdup( EMSG_SSL_SETUP_FAIL);
		}
	    }

            if( plan->out->debug_level >= DEBUG_MEDIUM3)
            {
                fprintf( plan->out->info_out, "%sSSL init, rc=%d, context?=%d\n",
                  plan->disp->line_pref, *rc, !!fetch->ssl_context);
	    }
	}
        else
        {
            if( plan->out->debug_level >= DEBUG_MEDIUM3)
            {
                fprintf( plan->out->info_out, "%sSSL init, skipped, use-ssl?=%d, context?=%d\n",
                  plan->disp->line_pref, req->use_ssl, !!fetch->ssl_context);
	    }
	}
    }

    return;
}
