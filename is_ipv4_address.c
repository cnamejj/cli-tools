#include <arpa/inet.h>

int is_ipv4_address( char *ip)

{
    int result = 0, sysrc;
    struct sockaddr_in dest;

    if( ip) if( *ip)
    {
        dest.sin_family = AF_INET;
        dest.sin_port = 0;
        dest.sin_addr.s_addr = 0;

        sysrc = inet_pton( AF_INET, ip, &dest.sin_addr);
        if( sysrc == 1) result = 1;
    }

    return( result);
}
