#include "http-fetch.h"
#include "err_ref.h"

/* --- */

void ssl_handshake( int *rc, struct plan_data *plan)

{
    int sock, io_rc, err = 0, done = 0, ret, sslerr;
    struct target_info *target;
    struct fetch_status *fetch;
    struct exec_controls *runex = 0;
    SSL *ssl;

    if( *rc == RC_NORMAL)
    {
        target = plan->target;
        fetch = plan->status;
        runex = plan->run;

        if( target->use_ssl)
        {
            sock = fetch->conn_sock;
            ssl = map_sock_to_ssl( sock, fetch->ssl_context, bio_ssl_callback);
            if( ssl) fetch->ssl_box = ssl;
            else
            {
                err = 1;
                *rc = ERR_SSL_ERROR;
                (void) stash_ssl_err_info( fetch, ERR_get_error());
                if( !fetch->err_msg) fetch->err_msg = strdup( EMSG_SSL_SETUP_FAIL);
	    }

/* NOTE: timeout remainder should be re-calc'd each time through the loop 
 * but leaving it simple for now, until the basic logic is working.
 */
            for( ; !done && !err; )
            {
                io_rc = SSL_connect( ssl);
                if( io_rc == 1) done = 1;
                else
                {
                    ret = handle_ssl_error( &sslerr, ssl, io_rc, sock, runex->conn_timeout);
                    if( sslerr != SSLACT_RETRY && sslerr != SSLACT_READ && sslerr != SSLACT_WRITE)
                    {
                        done = 1;
                        err = 1;
                        *rc = ret;
                        (void) stash_ssl_err_info( fetch, ERR_get_error());
                        if( !fetch->err_msg) fetch->err_msg = strdup( EMSG_SSL_CONN_FAIL);
		    }
		}
	    }
	}
    }

    return;
}
