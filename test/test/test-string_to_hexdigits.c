#include <stdio.h>
#include <stdlib.h>

int main( int narg, char **opts)

{
    char *st = 0;

    if( narg < 2) printf( "No string given on the command line.\n");
    else
    {
        for( st = opts[ 1]; *st; st++) printf( "%02x", *st);
        printf( "\n");
    }

    exit( 0);
}
        
    
