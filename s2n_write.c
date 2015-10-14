#ifdef S2N_SUPPORT
#include "http-fetch.h"
#include "cli-sub.h"
#include "err_ref.h"

/* --- */

int s2n_write( int *rc, struct fetch_status *fetch, int timeout, char *buff, int blen)

{
    int io_rc, pend, sock, sysrc;
    struct s2n_connection *conn;
    s2n_blocked_status blocked;
    time_t now, deadline;

    ENTER( "s2n_write")
    if( *rc == RC_NORMAL)
    {
        conn = fetch->s2n_conn;
        sock = fetch->conn_sock;

        now = time( 0);
        deadline = now + (timeout / 1000);
        if( deadline < now) deadline = now + 1;

        for( pend = 1; pend && now <= deadline; )
        {
            io_rc = s2n_send( conn, buff, blen, &blocked);
// printf("dbg:: s2n_write: s2n_send() rc=%d, bl=%d, len=%d\n", io_rc, blocked, blen);

            if( blocked == S2N_BLOCKED_ON_WRITE)
            {
                sysrc = wait_until_sock_ready( sock, POLL_EVENTS_WRITE, timeout);
                if( !sysrc)
                {
                    pend = 0;
                    *rc = ERR_POLL_TIMEOUT;
		}
                else if( sysrc == -1)
                {
                    pend = 0;
                    *rc = ERR_POLL_FAILED;
		}

                now = time( 0);
// sleep(1);
            }

            else pend = 0;
        }
    }

// printf( "dbg:: s2n_write: rc=%d\n", io_rc);
    LEAVE( "s2n_write")
    return( io_rc);
}

#else

#define UNAVAILABLE_SN2_WRITE

#endif
