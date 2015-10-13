#include "cli-sub.h"
#include "http-fetch.h"
#include "err_ref.h"

#include <openssl/ssl.h>

/* --- */

void setup_ssl_env( int *rc, struct plan_data *plan)

{
    int sysrc;
    unsigned long hold_err;
    struct target_info *req = 0;
    struct fetch_status *fetch;
#ifdef S2N_SUPPORT
    struct s2n_config *config;
#endif

    if( *rc == RC_NORMAL)
    {
        fetch = plan->status;
        if( plan->redirect) if( plan->redirect->conn_url) if( *plan->redirect->conn_url) req = plan->redirect;
        if( !req) req = plan->target;

        if( !req || !fetch) *rc = ERR_UNSUPPORTED;

        else if( req->use_ssl && !req->use_s2n && !fetch->ssl_context)
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

#ifdef S2N_SUPPORT
        else if( req->use_ssl && req->use_s2n && !fetch->s2n_conn)
        {
            sysrc = setenv( SN_ENABLE_CLIENT, IS_ONE_STR, 1);
            if( !sysrc) sysrc = setenv( SN_DISABLE_MLOCK, IS_ONE_STR, 1);
            if( !sysrc) sysrc = s2n_init();
            if( sysrc) *rc = ERR_SSL_INIT_FAILED;
            else
            {
                config = s2n_config_new();
                s2n_config_set_cipher_preferences( config, "default");
                fetch->s2n_conn = s2n_connection_new( S2N_CLIENT);
	    }
	}
#endif

        else
        {
            if( plan->out->debug_level >= DEBUG_MEDIUM3)
            {
                fprintf( plan->out->info_out, "%sSSL init, skipped, use-ssl?=%d, use-s2n?=%d, context?=%d, s2nconn?=%d\n",
                  plan->disp->line_pref, req->use_ssl, req->use_s2n, !!fetch->ssl_context, !!fetch->s2n_conn);
	    }
	}
    }

    return;
}
