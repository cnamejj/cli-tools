#include <string.h>
#include <errno.h>
#include "../cli-sub.h"

/* --- */

#define ERROR_EXIT( etype, specific) \
{ \
    fprintf( stderr, "**Error(%d)** Call to '%s' failed.\n", specific, etype); \
}

/* --- */

int main( int narg, char **opts)

{
    int rc;
    char *env_server_name = "SERVER_NAME", *env_server_val = "ServerTest",
      *env_qstring_name = "QUERY_STRING", *env_qstring_val = "url=www.cnn.com&packetime=yes&loop=2";
    char *cgi_data = 0;

    /* --- */

    printf( "Test: query string set\n");

    rc = setenv( env_server_name, env_server_val, 1);
    if( rc) ERROR_EXIT( "setenv", errno)

    rc = setenv( env_qstring_name, env_qstring_val, 1);
    if( rc) ERROR_EXIT( "setenv", errno)

    cgi_data = get_cgi_data( &rc);

    printf( "rc=%d", rc);
    if( cgi_data) printf( " len=%lu data(%s)\n", (unsigned long) strlen( cgi_data), cgi_data);

    /* --- */

    printf( "\nTest: no query string\n");

    rc = unsetenv( env_qstring_name);
    if( rc) ERROR_EXIT( "unsetenv", errno)

    cgi_data = get_cgi_data( &rc);

    printf( "rc=%d", rc);
    if( cgi_data) printf( " len=%lu data(%s)\n", (unsigned long) strlen( cgi_data), cgi_data);

    /* --- */

    exit( 1);
}
