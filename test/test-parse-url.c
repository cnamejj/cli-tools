#include "../cli-sub.h"

#define SPSP( ST) ST ? ST : "\0"

int main( int narg, char **opts)

{
    struct url_breakout *parts = 0;

    if( narg < 2) printf( "Give a URL as the first command line argument.\n");
    else
    {
        parts = parse_url_string( opts[ 1]);
        if( !parts) printf( "Error: Null pointer returned.\n");
        else
        {
            printf( "Breakout for URL '%s'\n", opts[ 1]);
            printf( "-- status: %d\n", parts->status);
            printf( "-- port: %d\n", parts->port);
            printf( "-- protocol (%s)\n", SPSP( parts->protocol));
            printf( "-- user (%s)\n", SPSP( parts->user));
            printf( "-- target (%s)\n", SPSP( parts->target));
            printf( "-- host (%s)\n", SPSP( parts->host));
            printf( "-- ip4 (%s)\n", SPSP( parts->ip4));
            printf( "-- ip6 (%s)\n", SPSP( parts->ip6));
            printf( "-- uri (%s)\n", SPSP( parts->uri));
            printf( "-- query (%s)\n", SPSP( parts->query));
	}
    }

    exit( 0);
}
