#include <arpa/inet.h>

#define SCOPE_LINK 0x02

int is_ipv6_address( char *ip)

{
    int result = 0, sysrc;
    struct sockaddr_in6 dest;

    if( ip) if( *ip)
    {
        dest.sin6_family = AF_INET6;
        dest.sin6_port = 0;
        dest.sin6_flowinfo = 0;
        dest.sin6_addr = in6addr_loopback;
        dest.sin6_scope_id = SCOPE_LINK;

        sysrc = inet_pton( AF_INET6, ip, &dest.sin6_addr);
        if( sysrc == 1) result = 1;
    }

    return( result);
}
