#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#include <stdio.h>
#endif

/* --- */

#define SYSCALL_FAIL_TEMPLATE "A call to system routine %s() failed."
#define SYSCALL_FAIL_GENERIC "An unspecified system call failed."

/* --- */

char *sys_call_fail_msg( char *routine)

{
    int size = 0;
    char *errmsg = 0;

    if( routine) if( *routine)
    {
        size = strlen( SYSCALL_FAIL_TEMPLATE) + strlen( routine);
        errmsg = (char *) malloc( size);
        if( errmsg) snprintf( errmsg, size, SYSCALL_FAIL_TEMPLATE, routine);
    }
    
    if( !errmsg) errmsg = strdup( SYSCALL_FAIL_GENERIC);

    return( errmsg);
}
