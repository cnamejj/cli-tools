#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "cli-sub.h"
#include "err_ref.h"

/*
... need to deal with http://user@domain:port/uri-string?query-data
... as well, meaning the "user@" bit needs to be added

... need to come up with a way to put a port on an IPV6 address as well
... use a pound sign '#' as the delimiter
 */

/* --- */

struct url_breakout *alloc_url_breakout()

{
    struct url_breakout *parts = 0;

    parts = (struct url_breakout *) malloc( sizeof *parts);
    if( parts)
    {
        parts->status = URL_VALID;
        parts->port = NO_PORT;
        parts->protocol = 0;
        parts->user = 0;
        parts->target = 0;
        parts->host = 0;
	parts->ip4 = 0;
	parts->ip6 = 0;
	parts->uri = 0;
	parts->query = 0;
        parts->use_ssl = 0;
    }

    return( parts);
}

/* --- */

void free_url_breakout( struct url_breakout *parts)

{
    if( parts)
    {
        if( parts->protocol) free( parts->protocol);
        if( parts->user) free( parts->user);
        if( parts->target) free( parts->target);
        if( parts->host) free( parts->host);
	if( parts->ip4) free( parts->ip4);
	if( parts->ip6) free( parts->ip6);
	if( parts->uri) free( parts->uri);
	if( parts->query) free( parts->query);

        parts->protocol = 0;
        parts->user = 0;
        parts->target = 0;
        parts->host = 0;
        parts->ip4 = 0;
        parts->ip6 = 0;
        parts->uri = 0;
        parts->query = 0;

        free( parts);
    }

    return;
}

/* --- */

struct url_breakout *parse_url_string( char *url)

{
    int rc = RC_NORMAL, stlen, port_number = NO_PORT;
    char *copy = 0, *protocol = 0, *user = 0, *domain = 0, *port = 0,
      *uri = 0, *query = 0, *pos = 0, *en_protocol = 0, *en_user = 0, 
      *en_domain = 0, *st_port = 0, *st_query = 0, *next = 0, *scan_from = 0,
      *st_ipv6_port = 0, hold;
    struct url_breakout *parts = 0;

    copy = strdup( url);
    if( !copy) rc = ERR_MALLOC_FAILED;

    if( rc == RC_NORMAL)
    {
        parts = alloc_url_breakout();
        if( !parts) rc = ERR_MALLOC_FAILED;
    }

    if( rc == RC_NORMAL)
    {
        scan_from = pos = copy;
        next = copy + strlen( copy);

        en_protocol = strstr( scan_from, PROTOCOL_DELIM);
        if( en_protocol)
        {
            scan_from = en_protocol + strlen( PROTOCOL_DELIM);
            next = en_protocol;
	}

        en_user = strstr( scan_from, USER_DELIM);
        en_domain = strstr( scan_from, DOMAIN_DELIM);

        if( en_user)
        {
            if( !en_domain) scan_from = en_user + 1;
            else if( en_user < en_domain) scan_from = en_user + 1;
	}

        st_port = strstr( scan_from, PORT_DELIM);
        st_query = strstr( scan_from, QUERY_DELIM);

        st_ipv6_port = strstr( scan_from, IPV6_PORT_DELIM);
        if( st_ipv6_port)
        {
            if( en_domain)
            {
/* fixes the case:  http://2601:9:5480:7d:59a8:fa5e:a8d2:c72d#8080/something.html */
/* also allows:     http://www.foo.com#8080/something.html */
                if( st_ipv6_port < en_domain) st_port = st_ipv6_port;
	    }
            else if( st_query)
            {
/* fixes the case:  http://2601:9:5480:7d:59a8:fa5e:a8d2:c72d#8080?data=some */
/* also allows:     http://www.foo.com#8080?data=some */
                if( st_ipv6_port < st_query) st_port = st_ipv6_port;
	    }
/* fixes the case:  http://2601:9:5480:7d:59a8:fa5e:a8d2:c72d#8080 */
/* also allows:     http://www.foo.com#8080 */
            else st_port = st_ipv6_port;
	}

        if( en_user) if( en_user < next) next = en_user;
        if( en_domain) if( en_domain < next) next = en_domain;
        if( st_port) if( st_port < next) next = st_port;
	if( st_query) if( st_query < next) next = st_query;

    }

    if( rc == RC_NORMAL && next == en_protocol)
    {
        hold = *en_protocol;
        *en_protocol = '\0';
        protocol = strdup( pos);
        *en_protocol = hold;
        if( !protocol) rc = ERR_MALLOC_FAILED;
        else
        {
            pos = en_protocol + strlen( PROTOCOL_DELIM);

            next = pos + strlen( pos);
            if( en_user) if( en_user < next) next = en_user;
            if( en_domain) if( en_domain < next) next = en_domain;
            if( st_port) if( st_port < next) next = st_port;
            if( st_query) if( st_query < next) next = st_query;
        }
    }

    if( rc == RC_NORMAL && next == en_user)
    {
        hold = *en_user;
        *en_user = '\0';
        user = strdup( pos);
        *en_user = hold;
        if( !user) rc = ERR_MALLOC_FAILED;
        pos = en_user + 1;

        next = next + strlen( next);
        if( st_port) if( st_port < next) next = st_port;
        if( en_domain) if( en_domain < next) next = en_domain;
        if( st_query) if( st_query < next) next = st_query;
    }

    if( rc == RC_NORMAL && next == st_port)
    {
        /* This is annoying, but we need to see if the ":" we found was part
         * of an IPV6 address and not a ":port" suffixs on a domain or IPV4
         * address.
         */
        if( en_domain)
        {
            hold = *en_domain;
            *en_domain = '\0';
	}

        if( is_ipv6_address( pos))
        {
            st_port = 0;

            domain = strdup( pos);
            if( !domain) rc = ERR_MALLOC_FAILED;

            if( en_domain)
            {
                *en_domain = hold;
                pos = en_domain;
	    }
            else pos = pos + strlen( pos);

            next = pos;
	}
        else
        {
            if( en_domain) *en_domain = hold;

            hold = *st_port;
            *st_port = '\0';
            domain = strdup( pos);
            *st_port = hold;
            if( !domain) rc = ERR_MALLOC_FAILED;
            else
            {
                stlen = strlen( PORT_DELIM);
                pos = st_port + stlen;

                next = next + strlen( next);
                if( en_domain) if( en_domain < next) next = en_domain;
                if( st_query) if( st_query < next) next = st_query;

                hold = *next;
                *next = '\0';
                port = strdup( pos);
                *next = hold;
                if( !port) rc = ERR_MALLOC_FAILED;

                pos = next;
            }
	}
    }

    if( rc == RC_NORMAL && next == en_domain)
    {
        if( !domain)
        {
            hold = *en_domain;
            *en_domain = '\0';
            domain = strdup( pos);
            *en_domain = hold;
            if( !domain) rc = ERR_MALLOC_FAILED;
            pos = en_domain;
        }

        if( rc == RC_NORMAL)
        {
            if( !st_query)
            {
                uri = strdup( pos);
                pos = pos + strlen( pos);
                next = pos;
	    }
            else
            {
                hold = *st_query;
                *st_query = '\0';
                uri = strdup( pos);
                *st_query = hold;
                next = pos = st_query;
	    }

            if( !uri) rc = ERR_MALLOC_FAILED;
	}
    }
            
    if( rc == RC_NORMAL && next == st_query)
    {
        if( !domain)
        {
            hold = *st_query;
            *st_query = '\0';
            domain = strdup( pos);
            *st_query = hold;
            if( !domain) rc = ERR_MALLOC_FAILED;
	}

        if( rc == RC_NORMAL)
        {
            query = strdup( st_query + 1);
            if( !query) rc = ERR_MALLOC_FAILED;
            pos = pos + strlen( pos);
            next = pos;
	}
    }

    if( rc == RC_NORMAL && !domain && !*next && *pos)
    {
        domain = strdup( pos);
        if( !domain) rc = ERR_MALLOC_FAILED;
        pos = next;
    }

    if( rc == RC_NORMAL && port)
    {
        if( !isdigit( *port)) rc = ERR_INVALID_DATA;
        else
        {
            errno = 0;
            port_number = strtoul( port, &pos, BASE10);
            if( errno) rc = ERR_INVALID_DATA;
            else if( *pos) rc = ERR_INVALID_DATA;
	}
    }

    if( rc == RC_NORMAL && domain)
    {
        pos = strdup( domain);
        if( !pos) rc = ERR_MALLOC_FAILED;
        else if( is_ipv4_address( domain)) parts->ip4 = pos;
        else if( is_ipv6_address( domain)) parts->ip6 = pos;
        else parts->host = pos;
    }

    if( rc == RC_NORMAL)
    {
        if( !domain) parts->status = URL_ERROR;
        else if( !*domain) parts->status = URL_ERROR;
        else
        {
            parts->status = URL_VALID;
            parts->port = port_number;
            parts->protocol = protocol;
            parts->user = user;
            parts->target = domain;
            parts->uri = uri;
            parts->query = query;
            if( protocol) parts->use_ssl = !strcasecmp( protocol, SSL_PROT_PREFIX);

            protocol = user = domain = uri = query = 0;
	}
    }
    else if( parts)
    {
        parts->status = URL_ERROR;
    }

    if( protocol) free( protocol);
    if( user) free( user);
    if( domain) free( domain);
    if( uri) free( uri);
    if( query) free( query);
    if( port) free( port);
    if( copy) free( copy);

    return( parts);
}

