#ifdef S2N_SUPPORT
#include "http-fetch.h"
#include "cli-sub.h"
#include "err_ref.h"
#include <unistd.h>

/* --- */

int s2n_read( int *rc, struct fetch_status *fetch, int timeout, char *buff, int blen)

{
    int io_rc, pend, sock, sysrc;
    struct s2n_connection *conn;
    s2n_blocked_status blocked;
    time_t now, deadline;

    if( *rc == RC_NORMAL)
    {
        conn = fetch->s2n_conn;
        sock = fetch->conn_sock;

        now = time( 0);
        deadline = now + (deadline / 1000);
        if( deadline < now) deadline = now + 1;

        for( pend = 1; pend && now <= deadline; )
        {
            io_rc = s2n_recv( conn, buff, blen, &blocked);
/* printf( "dbg:: s2n_read: inner: rc=%d, len=%d, blocked=%d\n", io_rc, blen, blocked); */

            if( blocked == S2N_BLOCKED_ON_READ)
            {
                sysrc = wait_until_sock_ready( sock, POLL_EVENTS_READ, timeout);
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
printf( "dbg:: s2n_read: read-block: rc=%d\n", sysrc);
            }

            else pend = 0;

            now = time( 0);
        }
    }

/*
if( io_rc > 0) write(fileno(stdout), buff, io_rc);
printf( "dbg:: s2n_read: rc=%d\n", io_rc);
 */

    return( io_rc);
}

#else

#define UNAVAILABLE_SN2_READ

#endif
