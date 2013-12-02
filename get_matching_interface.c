#include <netinet/in.h>
#include <string.h>
#include <ifaddrs.h>

#include "cli-sub.h"

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
