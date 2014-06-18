#include "http-fetch.h"
#include "err_ref.h"

/* --- */

void ssl_handshake( int *rc, struct plan_data *plan)

{
    int sock, io_rc, err = 0, done = 0, ret, sslerr;
    unsigned long hold_err;
    struct target_info *targ = 0;
    struct fetch_status *fetch;
    struct exec_controls *runex = 0;
    SSL *ssl;

    if( *rc == RC_NORMAL)
    {
        fetch = plan->status;
        runex = plan->run;
        if( plan->redirect) if( plan->redirect->conn_url) if( *plan->redirect->conn_url) targ = plan->redirect;
        if( !targ) targ = plan->target;

        if( targ->use_ssl)
        {
            sock = fetch->conn_sock;
            ssl = map_sock_to_ssl( sock, fetch->ssl_context, bio_ssl_callback);
            if( ssl) fetch->ssl_box = ssl;
            else
            {
                err = 1;
                *rc = ERR_SSL_ERROR;
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

/* NOTE: timeout remainder should be re-calc'd each time through the loop 
 * but leaving it simple for now, until the basic logic is working.
 */
            for( ; !done && !err; )
            {
                io_rc = SSL_connect( ssl);
                if( io_rc == 1)
                {
                    done = 1;
                    *rc = capture_checkpoint( fetch, EVENT_SSL_HANDSHAKE);
                    if( *rc == RC_NORMAL) fetch->last_state |= LS_SSL_SHAKE_DONE;
		}
                else
                {
                    ret = handle_ssl_error( &sslerr, ssl, io_rc, sock, runex->conn_timeout);
                    if( sslerr != SSLACT_RETRY && sslerr != SSLACT_READ && sslerr != SSLACT_WRITE)
                    {
                        done = 1;
                        err = 1;
                        *rc = ret;
                        hold_err = ERR_peek_error();
                        if( hold_err)
                        {
                            fetch->end_errno = hold_err;
                            if( !fetch->err_msg) (void) stash_ssl_err_info( fetch, hold_err);
                        }
                        else
                        {
                            fetch->end_errno = errno;
                            fetch->err_msg = strdup( EMSG_SSL_CONN_FAIL);
                        }
		    }
		}
	    }
	}
    }

    return;
}
