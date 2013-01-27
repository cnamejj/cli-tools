#include <pwd.h>
#include <grp.h>
#include <string.h>

#include "cli-sub.h"
#include "err_ref.h"

/* --- */

char *get_groupname( int *rc, uid_t gid)

{
    struct group *gr = 0;
    char *result = 0;
    
    *rc = RC_NORMAL;

    gr = getgrgid( gid);
    if( !gr) *rc = ERR_SYS_CALL;
    else
    {
        result = strdup( gr->gr_name);
        if( !result) *rc = ERR_MALLOC_FAILED;
    }

    return( result);
}
