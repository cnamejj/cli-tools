#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "err_ref.h"
#include "cli-sub.h"

/* --- */

char *build_syscall_errmsg( char *syscall, int sysrc)

{
    int errlen = 0;
    char *errmsg = 0, *name = 0, *def_name = "unspecified system call";

    if( !syscall) name = def_name;
    else if( !*syscall) name = def_name;
    else name = syscall;

    errlen = strlen( ERRMSG_SYSCALL2_FAILED) + strlen( name) + INT_DISPLAY_LEN;
    errmsg = (char *) malloc( errlen);
    if( errmsg) snprintf( errmsg, errlen, ERRMSG_SYSCALL2_FAILED, name, sysrc);

    return( errmsg);
}
