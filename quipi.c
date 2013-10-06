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
#include "cli-sub.h"
#include "err_ref.h"

/* --- */

void quipi_exit( int rc, int is_cgi )

{
    if( is_cgi ) printf( "%s\n", HTML_RESP_CLOSE );

    exit( rc );
}


/* --- */

void display_entry_form()

{
    int complete = 0;
    char *server_name = 0, *server_port = 0, *script_name = 0, *sep = COLON_ST;

    server_name = getenv( ENV_SERVER_NAME);
    server_port = getenv( ENV_SERVER_PORT);
    script_name = getenv( ENV_SCRIPT_NAME);

    if( !server_name || !server_port || !script_name) complete = 0;
    else if( !*server_name || !*server_port || !*script_name) complete = 0;
    else
    {
        complete = 1;

        if( !strcmp( server_port, DEFAULT_HTTP_PORT))
        {
            sep = EMPTY_ST;
            server_port = EMPTY_ST;
	}
    }

    if( complete ) printf( HTTP_FORM_TEMPLATE, server_name, sep, server_port, script_name);
    else printf( "%s\n", HTTP_FORM_GEN_ERROR);

    /* --- */

    quipi_exit( RC_NORMAL, 1 );
}


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
    int rc = RC_NORMAL, seq = 0, debug_level = 0, show_help = 0, is_cgi = 0, do_form = 0,
      dbg_seq = 1;
    unsigned int family;
    char *st, *name, *canon, *strc, *cgi_data = 0, *en;
    char *unknown_name = strdup( NO_HOSTNAME ), *unknown_ip4 = strdup( NO_IP4_ADDR ),
      *unknown_ip6 = strdup( NO_IP6_ADDR );
    char display_ip[ INET6_ADDRSTRLEN+1 ];
    struct addrinfo *host_recs, *ai;
    struct sockaddr_in *saddr4;
    struct sockaddr_in6 *saddr6;
    struct option_set opset[] = {
      { OP_DEBUG,   OP_TYPE_INT,  OP_FL_BLANK, FL_DEBUG,     0, DEF_DEBUG,   0, 0 },
      { OP_HELP,    OP_TYPE_FLAG, OP_FL_BLANK, FL_HELP,      0, DEF_HELP,    0, 0 },
      { OP_LOOKUP,  OP_TYPE_CHAR, OP_FL_BLANK, FL_LOOKUP,    0, DEF_LOOKUP,  0, 0 },
    };
    struct option_set *co = 0;
    struct word_chain *extra_opts = 0, *walk = 0, *endlist = 0;
    int nflags = (sizeof opset) / (sizeof opset[0]);
    FILE *errout = stderr;

    /* --- */

    if( called_as_cgi())
    {
        is_cgi = 1;
        errout = stdout;
        printf( "%s\n", HTML_RESP_HEADER );
        cgi_data = get_cgi_data( &rc);
        if( rc == RC_NORMAL) extra_opts = parse_cgi_options( &rc, opset, nflags, cgi_data);
        printf( "<!-- CGI data: (%s) -->\n", cgi_data);
    }
    else
    {
        if( narg < 2) show_help = 1;

        extra_opts = parse_command_options( &rc, opset, nflags, narg, opts);
    }

    if( rc != RC_NORMAL)
    {
        fprintf( errout, "Error parsing command options, rc=%d\n", rc);
        quipi_exit( 1, is_cgi);
    }

    /* --- */

    co = get_matching_option( OP_DEBUG, opset, nflags);
    if( !co)
    {
        fprintf( errout, "Program error dealing with options, this should never happen.\n");
        quipi_exit( 1, is_cgi);
    }
    debug_level = *((int *) co->parsed);

    co = get_matching_option( OP_HELP, opset, nflags);
    if( !co)
    {
        fprintf( errout, "Program error dealing with options, this should never happen.\n");
        quipi_exit( 1, is_cgi);
    }
    if( co->flags & OP_FL_SET) show_help = 1;

    co = get_matching_option( OP_LOOKUP, opset, nflags);
    if( !co)
    {
        fprintf( errout, "Program error dealing with options, this should never happen.\n");
        quipi_exit( 1, is_cgi);
    }
    st = (char *) co->parsed;
    if( st) if( *st)
    {
        for( en = st; *en; en++)
          if( *en == TAB_CH || *en == CR_CH || *en == LF_CH) *en = BLANK_CH;

        printf( "<!-- cleaned: (%s) -->\n", st);

        walk = extra_opts;
        if( walk) for( ; walk->next; walk = walk->next ) ;

        for( ; *st; )
        {
            en = index( st, BLANK_CH);
            if( en) *en = EOS_CH;

            printf( "<!-- %d. (%s) -->\n", dbg_seq++, st);                
            endlist = add_to_word_chain( endlist, st);
            if( !endlist)
            {
                fprintf( errout, "Program error, unable to allocate memory.\n");
                quipi_exit( 1, is_cgi);
	    }

            if( !walk) walk = endlist;
            else if( !walk->next) walk->next = endlist;
            if( en)
            {
                for( en++; *en == BLANK_CH ; en++) ;
                st = en;
	    }
            else for( ; *st; st++) ;
	}

        if( !extra_opts) extra_opts = walk;
    }

    /* --- */

    if( show_help)
    {
        st = opts[ 0];
        if( *st == '.' && *(st + 1) == '/') st += 2;
        printf( MSG_SHOW_SYNTAX, st);
        quipi_exit( 1, is_cgi);
    }

    /* --- */

    memset( display_ip, '\0', (sizeof display_ip) );

    /* --- */

    if( !is_cgi && narg<=1 )
    {
        st = opts[0];
        if( *st == '.' && *(st+1) == '/') st += 2;
        printf( "Syntax: %s [ --debug ## ] [ host1 | ip1 ] [ host2 | ip2 ] ...\n", st );
    }

    if( is_cgi)
    {
        if( !extra_opts) do_form = 1;
        else if( !extra_opts->opt) do_form = 1;
        else if( !*extra_opts->opt) do_form = 1;

        if( do_form)
        {
            display_entry_form();

            quipi_exit( RC_NORMAL, is_cgi );
	}
    }

    /* --- */

    printf( "%s", HTML_RESP_START);

    for( walk = extra_opts; walk; walk = walk->next )
    {
        name = walk->opt;

        host_recs = convert_from_ip_addr( name, debug_level );
        if( !host_recs ) host_recs = convert_from_hostname( name, debug_level );

        if( !host_recs)
        {
            fprintf( errout, "**Err** Name '%s' is unresolvable, doesn't look like an IP address or a hostname.\n", name);
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

    quipi_exit( rc, is_cgi );

    exit( 0);
}
