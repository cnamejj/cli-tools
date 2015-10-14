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

    ENTER( "s2n_finish")
    conn = fetch->s2n_conn;

    /* Todo: Need to check the code to see how 'blocked' is set */
    s2n_shutdown( conn, &blocked);

    /* -- Seems like making this call blows things up between client runs.
     * -- Disabling for now, but wind up adding it back or moving it to
     * -- the exit of the program (after all iterations) instead.  Need to
     * -- review the code to figure it out.
     * s2n_connection_free( conn);
     */

    LEAVE( "s2n_finish")
    return( rc);
}

#else

#define UNAVAILABLE_SN2_FINISH

#endif
