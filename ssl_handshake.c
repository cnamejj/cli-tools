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
// printf("dbg:: ssl-handshake: map failed\n");
	    }

/* NOTE: timeout remainder should be re-calc'd each time through the loop 
 * but leaving it simple for now, until the basic logic is working.
 */
            for( ; !done && !err; )
            {
                io_rc = SSL_connect( ssl);
// printf("dbg:: ssl-handshake: ssl-conn, rc=%d RESULT\n", io_rc);
                if( io_rc == 1) done = 1;
                else
                {
// const char *qerr_file;
// unsigned long qerr;
// int qerr_line;
// 
// qerr_file = (char *) malloc(2048);
// 
// qerr = ERR_get_error_line(&qerr_file, &qerr_line);
// for(; qerr; )
// {
//     printf("dbg:: ErrQueue: rc=%ld, line=%d, file'%s'\n", qerr, qerr_line, qerr_file);
// qerr = ERR_get_error_line(&qerr_file, &qerr_line);
// }

                    ret = handle_ssl_error( &sslerr, ssl, io_rc, sock, runex->conn_timeout);
// printf("dbg:: ssl-handshake: hse, rc=%d, serr=%d\n", ret, sslerr);
                    if( sslerr != SSLACT_RETRY && sslerr != SSLACT_READ && sslerr != SSLACT_WRITE)
                    {
                        done = 1;
                        err = 1;
                        *rc = ret;
		    }
		}
	    }
	}
    }

// printf("dbg:: ssl-handshake: leaving... rc=%d err=%d\n", *rc, err);

    return;
}
