#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* IFF_UP flag */

struct interface_info {
    char *name;
    unsigned int flags;
    struct sockaddr *addr;
};

struct interface_info *get_matching_interface( const char *if_name, int protocol, unsigned int flags);

struct interface_info *get_matching_interface( const char *if_name, int protocol, unsigned int flags)

{
    int rc, sa, hit = 0;
    struct ifaddrs *ifaddr = 0, *walk = 0;
    struct interface_info *result = 0;
    struct sockaddr_in *sock4 = 0;
    struct sockaddr_in6 *sock6 = 0;

    rc = getifaddrs( &ifaddr);

    if( rc != -1)
    {
        for( walk = ifaddr; walk && !hit; walk = walk->ifa_next)
        {
            if( walk->ifa_addr)
            {
                sa = walk->ifa_addr->sa_family;

                if( protocol == AF_UNSPEC && (sa == AF_INET || sa == AF_INET6)) hit = 1;
                else if( sa == protocol) hit = 1;
                else hit = 0;

                if( hit && if_name)
                {
                    if( *if_name) hit = !strcmp( walk->ifa_name, if_name);
		}

                if( hit && flags) hit = flags & walk->ifa_flags;

                if( hit)
                {
                    result = (struct interface_info *) malloc( sizeof *result);
                    if( result)
                    {
                        result->flags = walk->ifa_flags;
                        result->name = strdup( walk->ifa_name);
                        result->addr = 0;

                        if( sa == AF_INET)
                        {
                            sock4 = (struct sockaddr_in *) malloc( sizeof *sock4);
                            if( sock4) memcpy( sock4, walk->ifa_addr, sizeof *sock4);
                            result->addr = (struct sockaddr *) sock4;
			}
                        else if( sa == AF_INET6)
                        {
                            sock6 = (struct sockaddr_in6 *) malloc( sizeof *sock6);
                            if( sock6) memcpy( sock6, walk->ifa_addr, sizeof *sock6);
                            result->addr = (struct sockaddr *) sock6;
			}

                        if( !result->name || !result->addr)
                        {
                            if( result->name) free( result->name);
                            if( result->addr) free( result->addr);
                            free( result);
                            result = 0;
			}
		    }
		}
            }
	}
    }

    if( ifaddr) freeifaddrs( ifaddr);

    return( result);
}

int main( int narg, char **opts)

{
    int sa, protocol = AF_UNSPEC;
    const char *strc = 0, *patt = 0;
    char display_ip[ INET6_ADDRSTRLEN + 1];
    struct interface_info *match = 0;
    struct sockaddr_in *sock4 = 0;
    struct sockaddr_in6 *sock6 = 0;

    if( narg > 1) patt = opts[ 1];

    if( narg > 2)
    {
        if( !strcmp( opts[ 2], "4")) protocol = AF_INET;
        else if( !strcmp( opts[ 2], "6")) protocol = AF_INET6;
    }

    memset( display_ip, '\0', sizeof display_ip);

    match = get_matching_interface( patt, protocol, IFF_UP);
    if( match)
    {
        sa = match->addr->sa_family;

        printf( "Interface (%s)\n", match->name);

        if( sa == AF_INET6)
        {
            sock6 = (struct sockaddr_in6 *) match->addr;
            strc = inet_ntop( sa, &sock6->sin6_addr, display_ip, sizeof display_ip);
            if( !strc) strcat( display_ip, "N/A(v6)");
            printf( "It's an IPv6 interface, address (%s)\n", display_ip);
        }
        else if( sa == AF_INET)
        {
            sock4 = (struct sockaddr_in *) match->addr;
            strc = inet_ntop( sa, &sock4->sin_addr, display_ip, sizeof display_ip);
            if( !strc) strcat( display_ip, "N/A(v4)");
            printf( "It's an IPv4 interface, address (%s)\n", display_ip);
        }
#ifndef __APPLE__
        else if( sa == AF_PACKET)
        {
            printf( "It's a PACKET interface\n");
        }
#endif
        else printf( "Socket Family %d\n", sa);
    }

    exit( 0);
}
