#include "cli-sub.h"

/* --- */

int wait_until_sock_ready( int sock, int event, int timeout)

{
    int rc = 0;
    struct pollfd pset;

    pset.fd = sock;
    pset.events = event;
    pset.revents = 0;
    
    rc = poll( &pset, 1, timeout);
// printf( "dbg:: wait-until-sock-ready: rc=%d, ev=%d, sock=%d\n", rc, event, sock);

    return(rc);
}
