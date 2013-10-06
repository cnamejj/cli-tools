#include "../cli-sub.h"

int main( int narg, char **opts)

{
    int result;

    if( narg < 2) printf( "Give an IP address string to test on the command line.\n");
    else
    {
        result = is_ipv6_address( opts[ 1]);
        printf( "Is '%s' an IPV6 address?  rc=%d\n", opts[ 1], result);
    }

    exit( 0);
}
