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
                ret = ERR_SSL_ERROR;
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
                    if( ret == RC_NORMAL) done = 1;
                    else if( sslerr != SSLERR_RETRY)
                    {
                        done = 1;
                        err = 1;
                        *rc = ret;
		    }
		}
	    }
	}
    }

    return;
}
