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

    return(rc);
}
