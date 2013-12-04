#include <stdio.h>
#include <ctype.h>

#include "../cli-sub.h"

int main( int narg, char **opts)

{
    unsigned int spot;

    for( spot = getc( stdin); spot != EOF; spot = getc( stdin))
    {
        spot &= 0xff;
        if( isprint( spot)) printf( "ch(%c)", spot);
        else printf( "'%02x'x", spot);
        printf( " res: %d\n", is_reserved_uri_char( spot));
    }

    exit( 0);
}
