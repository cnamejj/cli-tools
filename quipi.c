/* -----------
 * QuickIPInfo - qipi
 * ----
 * Do DNS lookups for all the arguments passed on the command line
 */

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "quipi.h"

/* --- */

void init_addrinfo_data( struct addrinfo *ai )

{
    ai->ai_flags = AI_CANONNAME;
    ai->ai_family = AF_UNSPEC;
    ai->ai_socktype = 0;
    ai->ai_protocol = 0;
    ai->ai_addrlen = 0;
    ai->ai_addr = 0;
    ai->ai_canonname = 0;
    ai->ai_next = 0;

    return;
}

/* --- */

void free_addrinfo_data( struct addrinfo *ai )

{
    struct addrinfo *walk, *next;

    for( walk = ai; walk; )
    {
        next = walk->ai_next;
        if( walk->ai_canonname ) free( walk->ai_canonname );
        if( walk->ai_addr ) free( walk->ai_addr );
        free( walk );
        walk = next;
    }

    return;
}

/* --- */

struct addrinfo *get_addrinfo_data( int sock_type )

{
    int ai_size;
    struct addrinfo *ai;

    ai = (struct addrinfo *) malloc( (sizeof *ai) );
    if( ai )
    {
        init_addrinfo_data( ai );

        if( sock_type == AF_INET6 ) ai_size = (sizeof (struct sockaddr_in6));
        else ai_size = (sizeof (struct sockaddr_in));

        ai->ai_addr = (struct sockaddr *) malloc( ai_size );
        if( ai->ai_addr )
        {
            ai->ai_addrlen = ai_size;
            memset( ai->ai_addr, '\0', ai_size);
        }
        else
        {
            free( ai );
            ai = 0;
	}
    }

    return( ai );
}

/* --- */

struct addrinfo *convert_from_ip_addr( char *name )

{
    int rc = 0, getrc = 0;
    char display_ip[ INET6_ADDRSTRLEN+1 ], *dis;
    struct addrinfo *result = 0;
    struct sockaddr_in *sa4;
    struct sockaddr_in6 *sa6;

    result = get_addrinfo_data( AF_INET);
    if( !result) rc = errno;
    sa4 = (struct sockaddr_in *) result->ai_addr;

    memset( display_ip, '\0', (sizeof display_ip) );
    dis = display_ip;

    if( inet_pton( AF_INET, name, &sa4->sin_addr ) )
    {
        sa4->sin_family = AF_INET;
        sa4->sin_port = 0;
        result->ai_family = AF_INET;

        getrc = getnameinfo( result->ai_addr, result->ai_addrlen, dis, (sizeof display_ip), NULL, 0, NI_NAMEREQD );
        if( !getrc)
        {
            if( result->ai_canonname) free( result->ai_canonname );
            result->ai_canonname = strdup( display_ip );
            if( !result->ai_canonname ) rc = errno;
	}
    }

    else
    {
        free_addrinfo_data( result);

        result = get_addrinfo_data( AF_INET6);
        if( result) sa6 = (struct sockaddr_in6 *) result->ai_addr;

        if( !result) rc = errno;
        else if( inet_pton( AF_INET6, name, &sa6->sin6_addr ) )
        {
            sa6->sin6_family = AF_INET6;
            sa6->sin6_port = 0;
            result->ai_family = AF_INET6;

            getrc = getnameinfo( result->ai_addr, result->ai_addrlen, dis, (sizeof display_ip), NULL, 0, NI_NAMEREQD );
            if( !getrc)
            {
                if( result->ai_canonname ) free( result->ai_canonname );
                result->ai_canonname = strdup( display_ip );
                if( !result->ai_canonname ) rc = errno;
            }
	}
        else
        {
            free_addrinfo_data( result);
            result = 0;
	}
    }

    if( rc || getrc == EAI_MEMORY )
    {
        fprintf( stderr, "**Err(%d)** Can't allocate data structure. (%s)\n", errno, strerror( errno ) );
        free_addrinfo_data( result );
    }
    else if( getrc )
    {
        if( result->ai_canonname ) free( result->ai_canonname );

        if( getrc == EAI_NONAME ) result->ai_canonname = strdup( LOOKUP_NONAME );
        else result->ai_canonname = strdup( LOOKUP_FAILED );

/* fprintf( stderr, "**Err(%d)** Message is (%s)\n", getrc, gai_strerror(getrc)); */
    }

    return( result);
}

/* --- */

struct addrinfo *convert_from_hostname( char *name )

{
    int chk, lookup = 1, getrc;
    char display_ip[ INET6_ADDRSTRLEN+1 ], *dis;
    struct addrinfo *host_recs = 0, hints, *ai = 0;

    /* --- */

    init_addrinfo_data( &hints );

    chk = getaddrinfo( name, NULL, &hints, &host_recs );
    if( !chk )
    {
        dis = display_ip;

        for( ai = host_recs; ai; ai = ai->ai_next )
        {
            if( !ai->ai_canonname) lookup = 1;
            else lookup = !*ai->ai_canonname;

            if( lookup)
            {
                memset( display_ip, '\0', (sizeof display_ip) );
                getrc = getnameinfo( ai->ai_addr, ai->ai_addrlen, dis, 
                  (sizeof display_ip), NULL, 0, NI_NAMEREQD );
                if( !getrc) ai->ai_canonname = strdup( display_ip );
	    }
	}
    }
/*    else
    {
        fprintf( stderr, "**Err(%d)** Lookup failed for '%s' (%s).\n", chk,
          name, gai_strerror( chk ) );
    }
 */

    return( host_recs );
}

/* --- */

int main( int narg, char **opts )

{
    int rc = OK, np, seq = 0;
#ifdef SHOW_DNS_DUMP            
    unsigned int family;
#endif
    char *st, *name, *canon, *strc;
    char *unknown_name = strdup( NO_HOSTNAME ), *unknown_ip4 = strdup( NO_IP4_ADDR ),
      *unknown_ip6 = strdup( NO_IP6_ADDR );
    char display_ip[ INET6_ADDRSTRLEN+1 ];
    struct addrinfo *host_recs, *ai;
    struct sockaddr_in *saddr4;
    struct sockaddr_in6 *saddr6;

    /* --- */

    memset( display_ip, '\0', (sizeof display_ip) );

    /* --- */

    if( narg<=1 )
    {
        st = opts[0];
        if( *st == '.' && *(st+1) == '/') st += 2;
        printf( "Syntax: %s [ host1 | ip1 ] [ host2 | ip2 ] ...\n", st );
    }

    /* --- */

    for( np=1; np<narg; np++ )
    {
        name = opts[np];

        host_recs = convert_from_ip_addr( name );
        if( !host_recs ) host_recs = convert_from_hostname( name );

        if( !host_recs)
        {
            fprintf( stderr, "**Err** Name '%s' is unresolvable, doesn't look like an IP address or a hostname.\n", name);
	}

        else
        {
            for( ai = host_recs; ai; ai = ai->ai_next )
            {
                canon = ai->ai_canonname;
                if( !canon) canon = unknown_name;
                else if( !*canon) canon = unknown_name;

                saddr4 = 0;
                saddr6 = 0;

                if( ai->ai_family == AF_INET)
                {
#ifdef SHOW_DNS_DUMP            
                    family = ADDR_IPV4;
#endif
                    saddr4 = (struct sockaddr_in *) ai->ai_addr;
                    st = display_ip;
                    strc = (char *) inet_ntop( AF_INET, &saddr4->sin_addr, st, (sizeof display_ip) );
                    if( !strc) strcat( display_ip, unknown_ip4 );
                }
                else if( ai->ai_family == AF_INET6)
                {
#ifdef SHOW_DNS_DUMP            
                    family = ADDR_IPV6;
#endif
                    saddr6 = (struct sockaddr_in6 *) ai->ai_addr;
                    st = display_ip;
                    strc = (char *) inet_ntop( AF_INET6, &saddr6->sin6_addr, st, (sizeof display_ip) );
                    if( !strc) strcat( display_ip, unknown_ip6 );
                }
                else
                {
#ifdef SHOW_DNS_DUMP            
                    family = ADDR_UNRECOG;
#endif
                    strcat( display_ip, unknown_ip4);
                }

#ifdef SHOW_DNS_DUMP            
                printf( "---------: %s #%d\n", name, ++seq );
                printf( "Flags____: %x\n", ai->ai_flags );
                printf( "Family___: %d : %d\n", ai->ai_family, family );
                printf( "SockType_: %d\n", ai->ai_socktype );
                printf( "Protocol_: %d\n", ai->ai_protocol );
                printf( "AddrLen__: %d\n", ai->ai_addrlen );
                printf( "*SockAddr: %lx (%s)\n", (long int) ai->ai_addr, display_ip );
                printf( "CanonName: %s\n", canon );
                printf( "Next_____: %lx\n", (long int) ai->ai_next );
#else
                printf( "%d. %s %s %s\n", ++seq, name, display_ip, canon );
#endif
            }

            freeaddrinfo( host_recs );
	}
    }

    /* --- */

    exit( rc );
}
