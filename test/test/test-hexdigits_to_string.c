#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "cli-sub.h"

int main( int narg, char **opts)

{
    int rc = 0, msglen, off;
    char *hex = 0, *msg = 0, *st = 0;

    if( narg < 2) printf( "No hex string on the command line.\n");
    else
    {
        hex = opts[ 1];
        msg = hexdigits_to_string( &rc, &msglen, hex);
        if( rc) printf( "Error %d\n", rc);
        else if( msglen < 1) printf( "Length %d\n", msglen);
        else
        {
            printf( "Msg %d: (", msglen);
            st = msg;
            for( off=0; off<msglen; off++)
            {
                if( isprint( *st)) printf( "%c", *st);
                else printf( "?");
                st++;
	    }
            printf( ")\n");
	}
    }

    exit( 0);
}
