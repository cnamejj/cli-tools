#define _GNU_SOURCE

#include <unistd.h>
#include <sys/types.h>
#include <grp.h>

#include "err_ref.h"
#include "cli-sub.h"

/* --- */

int switch_run_group( char *new_group)

{
    int rc = RC_NORMAL, sysrc;
    struct group *gr = 0;
    uid_t my_gid, my_egid, new_gid;

    /* --- */

    my_gid = getgid();
    my_egid = getegid();

    gr = getgrnam( new_group);
    if( !gr) rc = ERR_SYS_CALL;
    else
    {
        new_gid = gr->gr_gid;

        if( my_gid != new_gid || my_egid != new_gid)
        {
            sysrc = setresgid( new_gid, new_gid, new_gid);
            if( sysrc) rc = ERR_SYS_CALL;
        }
    }

    /* --- */

    return( rc);
}
