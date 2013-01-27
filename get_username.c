#include <pwd.h>
#include <string.h>

#include "err_ref.h"

/* --- */

char *get_username( int *rc, uid_t uid)

{
    struct passwd *pw = 0;
    char *result = 0;
    
    *rc = RC_NORMAL;

    pw = getpwuid( uid);
    if( !pw) *rc = ERR_SYS_CALL;
    else
    {
        result = strdup( pw->pw_name);
        if( !result) *rc = ERR_MALLOC_FAILED;
    }

    return( result);
}
