#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#endif

/* --- */

#define ERRMSG_WITH_STRING_GENERIC_MSG "An unspecified error occurred."

/* --- */

char *errmsg_with_string( const char *errmsg_template, const char *detail)

{
    int size = 0;
    char *msg = 0;

    if( errmsg_template && detail) if( *errmsg_template && *detail)
    {
        size = strlen( errmsg_template) + strlen( detail);
        msg = (char *) malloc( size);
        if( msg) snprintf( msg, size, errmsg_template, detail);
    }
    
    if( !msg) msg = strdup( ERRMSG_WITH_STRING_GENERIC_MSG);

    return( msg);
}
