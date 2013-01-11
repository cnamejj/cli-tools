/* -----------
 * QUickIPInfo - quipi
 * ----
 *
 * Do DNS lookups for the arguments passed on the command line, 
 * trying to figure out what type of "thing" was given without 
 * making the user specify.  The appropriate forward or backward
 * lookup is done, depending on what the code thinks it was given.
 */

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "quipi.h"
#include "parse_opt.h"
#include "err_ref.h"

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

struct addrinfo *convert_from_ip_addr( char *name, int debug )

{
    int rc = 0, getrc = 0;
    char display_host[ 256 ], *dho;
    char display_ip[ INET6_ADDRSTRLEN+1 ], *dis;
    struct addrinfo *result = 0;
    struct sockaddr_in *sa4;
    struct sockaddr_in6 *sa6;

    result = get_addrinfo_data( AF_INET);
    if( !result) rc = errno;
    sa4 = (struct sockaddr_in *) result->ai_addr;

    memset( display_ip, '\0', (sizeof display_ip) );
    dis = display_ip;

    memset( display_host, '\0', (sizeof display_host) );
    dho = display_host;

    if( inet_pton( AF_INET, name, &sa4->sin_addr ) )
    {
        sa4->sin_family = AF_INET;
        sa4->sin_port = 0;
        result->ai_family = AF_INET;

        getrc = getnameinfo( result->ai_addr, result->ai_addrlen, dho, (sizeof display_host), NULL, 0, NI_NAMEREQD );
        if( !getrc)
        {
            if( result->ai_canonname) free( result->ai_canonname );
            result->ai_canonname = strdup( display_host );
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

        if( debug >= DEBUG_MEDIUM1) fprintf( stderr, "**Err(%d)** Call to getnameinfo() failed, message (%s)\n", getrc, gai_strerror(getrc));
    }

    return( result);
}

/* --- */

struct addrinfo *convert_from_hostname( char *name, int debug )

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
    else if( debug >= DEBUG_MEDIUM1)
    {
        fprintf( stderr, "**Err(%d)** Call to getaddrinfo() failed for '%s', message (%s).\n", chk,
          name, gai_strerror( chk ) );
    }

    return( host_recs );
}

/* --- */

int main( int narg, char **opts )

{
    int rc = RC_NORMAL, seq = 0, debug_level = 0;
    unsigned int family;
    char *st, *name, *canon, *strc;
    char *unknown_name = strdup( NO_HOSTNAME ), *unknown_ip4 = strdup( NO_IP4_ADDR ),
      *unknown_ip6 = strdup( NO_IP6_ADDR );
    char display_ip[ INET6_ADDRSTRLEN+1 ];
    struct addrinfo *host_recs, *ai;
    struct sockaddr_in *saddr4;
    struct sockaddr_in6 *saddr6;
    struct option_set opset[] = {
      { OP_DEBUG,   OP_TYPE_INT,  OP_FL_BLANK, FL_DEBUG,     0, DEF_DEBUG,   0, 0 },
    };
    struct option_set *co = 0;
    struct word_chain *extra_opts = 0, *walk = 0;
    int nflags = (sizeof opset) / (sizeof opset[0]);

    /* --- */

    extra_opts = parse_command_options( &rc, opset, nflags, narg, opts);
    if( rc != RC_NORMAL)
    {
        fprintf( stderr, "Error parsing command options, rc=%d\n", rc);
        exit( 1);
    }

    co = get_matching_option( OP_DEBUG, opset, nflags);
    if( !co)
    {
        fprintf( stderr, "Program error dealing with options, this should never happen.\n");
        exit( 1);
    }
    debug_level = *((int *) co->parsed);

    /* --- */

    memset( display_ip, '\0', (sizeof display_ip) );

    /* --- */

    if( narg<=1 )
    {
        st = opts[0];
        if( *st == '.' && *(st+1) == '/') st += 2;
        printf( "Syntax: %s [ --debug ## ] [ host1 | ip1 ] [ host2 | ip2 ] ...\n", st );
    }

    /* --- */

    for( walk = extra_opts; walk; walk = walk->next )
    {
        name = walk->opt;

        host_recs = convert_from_ip_addr( name, debug_level );
        if( !host_recs ) host_recs = convert_from_hostname( name, debug_level );

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
                    family = ADDR_IPV4;
                    saddr4 = (struct sockaddr_in *) ai->ai_addr;
                    st = display_ip;
                    strc = (char *) inet_ntop( AF_INET, &saddr4->sin_addr, st, (sizeof display_ip) );
                    if( !strc) strcat( display_ip, unknown_ip4 );
                }
                else if( ai->ai_family == AF_INET6)
                {
                    family = ADDR_IPV6;
                    saddr6 = (struct sockaddr_in6 *) ai->ai_addr;
                    st = display_ip;
                    strc = (char *) inet_ntop( AF_INET6, &saddr6->sin6_addr, st, (sizeof display_ip) );
                    if( !strc) strcat( display_ip, unknown_ip6 );
                }
                else
                {
                    family = ADDR_UNRECOG;
                    strcat( display_ip, unknown_ip4);
                }

                seq++;
                if( debug_level >= DEBUG_HIGH1)
                {
                    printf( "---------: %s #%d\n", name, seq );
                    printf( "Flags____: %x\n", ai->ai_flags );
                    printf( "Family___: %d : %d\n", ai->ai_family, family );
                    printf( "SockType_: %d\n", ai->ai_socktype );
                    printf( "Protocol_: %d\n", ai->ai_protocol );
                    printf( "AddrLen__: %d\n", ai->ai_addrlen );
                    printf( "*SockAddr: %lx (%s)\n", (long int) ai->ai_addr, display_ip );
                    printf( "CanonName: %s\n", canon );
                    printf( "Next_____: %lx\n", (long int) ai->ai_next );
		}
                printf( "%d. %s %s %s\n", seq, name, display_ip, canon );
            }

            freeaddrinfo( host_recs );
	}
    }

    /* --- */

    exit( rc );
}
