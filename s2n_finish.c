#ifdef S2N_SUPPORT
#include "http-fetch.h"
#include "cli-sub.h"
#include "err_ref.h"

/* --- */

int s2n_finish( struct fetch_status *fetch, int timeout)

{
    int rc = RC_NORMAL;
    struct s2n_connection *conn;
    s2n_blocked_status blocked;

    conn = fetch->s2n_conn;

    /* Todo: Need to check the code to see how 'blocked' is set */
    s2n_shutdown( conn, &blocked);
    s2n_connection_free( conn);

    return( rc);
}

#else

#define UNAVAILABLE_SN2_FINISH

#endif
