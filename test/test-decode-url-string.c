#include <errno.h>
#include <string.h>
#include "../cli-sub.h"

int main( int narg, char **opts)

{
    char *result = 0;

    if( narg < 2)
    {
        fprintf( stderr, "**Error** Expected a string on the command line.\n");
        exit( 1);
    }

    result = decode_url_str( opts[ 1]);

    if( result) printf( "%s\n", result);
    else
    {
        fprintf( stderr, "**Error(%d)** Got a null string back...\n\t(%s)\n",
          errno, strerror( errno));
    }

    exit( 0);
}

    
