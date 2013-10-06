#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "../cli-sub.h"

/* --- */

#define ERR_EXIT( msg, vname, err) \
{ \
    fprintf( stderr, "**Error(%d)** %s failed for environment variable '%s'\n", err, msg, vname); \
}

/* --- */

int main( int narg, char **opts)

{
    int rc = 0, off;
    char *varname = 0, *varvalue = "Something";
    char *cgi_env_var[] = { "DOCUMENT_ROOT", "GATEWAY_INTERFACE", "HTTP_HOST",
      "PATH_INFO", "QUERY_STRING", "REQUEST_METHOD", "SERVER_ADDR",
      "SERVER_NAME", "SERVER_PROTOCOL", "SERVER_SOFTWARE" };

    rc = called_as_cgi();
    printf( "Asis: RC=%d\n", rc);

    for( off=0; off < (sizeof cgi_env_var) / (sizeof *cgi_env_var); off++)
    {
        varname = cgi_env_var[ off];
        rc = setenv( varname, varvalue, 1);
        if( rc) ERR_EXIT( "Set", varname, errno)

        rc = called_as_cgi();
        printf( "%s: RC=%d\n", varname, rc);

        rc = unsetenv( varname);
        if( rc) ERR_EXIT( "Unset", varname, errno)
    }

    exit( rc);
}
