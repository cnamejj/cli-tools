#include <stdio.h>
#include <ctype.h>

#include "../cli-sub.h"

int main( int narg, char **opts)

{
    unsigned int spot;

    for( spot = getc( stdin); spot != EOF; spot = getc( stdin))
    {
        spot &= 0xff;
/*        if( is_reserved_uri_char( spot)) printf( "&#%02x;", spot); */
        if( is_reserved_uri_char( spot)) printf( "%%%02x", spot);
        else printf( "%c", spot);
    }

    exit( 0);
}
