#define _GNU_SOURCE

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include "err_ref.h"
#include "cli-sub.h"

/* --- */

int switch_run_user( char *new_user)

{
    int rc = RC_NORMAL, sysrc;
    struct passwd *pw = 0;
    uid_t my_uid, my_euid, new_uid;

    /* --- */

    my_uid = getuid();
    my_euid = geteuid();

    pw = getpwnam( new_user);
    if( !pw) rc = ERR_SYS_CALL;
    else
    {
        new_uid = pw->pw_uid;

        if( my_uid != new_uid || my_euid != new_uid)
        {
            sysrc = setresuid( new_uid, new_uid, new_uid);
            if( sysrc) rc = ERR_SYS_CALL;
        }
    }

    /* --- */

    return( rc);
}
