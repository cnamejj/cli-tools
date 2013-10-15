#define TIME_SUMMARY_HEADER "\
-Date--TIme-     ----- Elapsed Time ---- Total  Bytes\n\
YrMnDyHrMnSe HRC   DNS  Conn 1stRD Close  Time   Xfer\n\
------------ --- ----- ----- ----- ----- ----- ------\n\
"

#define TIME_DISPLAY_FORMAT "%y%m%d%H%M%S"
#define TIME_DISPLAY_SIZE 15

/*
 * Bugs:
 * ---
 * - Use case: ./http-fetch -debug 1 -url www.nihilon.com -bogus
 * results in a malloc() failed error, which makes no sense.
 *
 * Features:
 * ---
 * - Add an option to control HTTP/1.0 or HTTP/1.1
 * - Understand and parse "chunked" content
 * - Allow saving raw or parsed (as in chunked) content
 * - Add an option to select IPv4 or IPv6 address from DNS results
 * - Let the user pick the bind IP by interface name (and prot 6/4 choice)
 * - Display both total bytes transfered and payload bytes with "-time" option
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <poll.h>
#ifndef linux
#include <fcntl.h>
#endif
#include <math.h>

#include "http-fetch.h"
#include "cli-sub.h"
#include "err_ref.h"

/* --- */

void debug_timelog( char *tag);

void debug_timelog( char *tag)

{
    static int seq = 0;
    long diff_sec, diff_sub, now_sec, now_sub, top = 1000000;
    static struct timeval now, prev = { 0, 0 };

    seq++;
    (void) gettimeofday( &now, 0);

    diff_sec = now.tv_sec - prev.tv_sec;
    diff_sub = now.tv_usec - prev.tv_usec;

    now_sec = now.tv_sec;
    now_sub = now.tv_usec;

    if( diff_sub < 0)
    {
        diff_sec--;
        diff_sub += top;
    }

    fprintf( stderr, "dbg:: %3d. %11ld.%06ld %11ld.%06ld (%s)\n", seq, now_sec, now_sub, diff_sec, diff_sub, tag);

    prev.tv_sec = now.tv_sec;
    prev.tv_usec = now.tv_usec;

    return;
}

/* --- */

float calc_time_difference( struct ckpt_entry *start, struct ckpt_entry *end, float frac_res)

{
    long diff_sec, diff_sub, top;
    float diff = 0.0;

    top = (long) (1.0 / frac_res);

    if( start && end)
    {
        diff_sec = end->sec - start->sec;
        diff_sub = end->frac - start->frac;

        if( diff_sub < 0)
        {
            diff_sec--;
            diff_sub += top;
        }

        diff = diff_sec + (frac_res * diff_sub);
    }

    return( diff);
}

/* --- */

int add_data_block( struct ckpt_chain *checkpoint, int len, char *buff)

{
    int rc = RC_NORMAL;
    struct data_block *packet = 0;

    if( checkpoint)
    {
        packet = (struct data_block *) malloc( sizeof *packet);
        if( !packet) rc = ERR_MALLOC_FAILED;
        else
        {
            packet->len = len;
            packet->data = (char *) malloc( len);
            if( !packet->data) rc = ERR_MALLOC_FAILED;
            else (void) memcpy( packet->data, buff, len);
	}
    }

    if( rc == RC_NORMAL) checkpoint->detail = packet;
    else
    {
        if( packet) free( packet);
    }

    return( rc);
}

/* --- */

void free_data_block( struct data_block *detail)

{
    if( detail)
    {
        if( detail->data) free( detail->data);
        free( detail);
    }

    return;
}

/* --- */

int find_connection( struct plan_data *plan)

{
    int rc = RC_NORMAL, uri_len, empty;
    char *proxy_req = 0, *url = 0;
    struct target_info *target = 0;
    struct url_breakout *parts = 0;
    struct fetch_status *status = 0;

    target = plan->target;
    proxy_req = target->proxy_url;
    url = target->conn_url;
    status = plan->status;

/* ... maybe if there's a proxy requested we can just override the "conn_host" and set a flag in "last_state"? ... */
    if( proxy_req) if( *proxy_req)
    {
        parts = parse_url_string( proxy_req);
        if( !parts) rc = ERR_MALLOC_FAILED;
        else if( parts->status == URL_VALID)
        {
            target->proxy_host = parts->host;
            target->proxy_ipv4 = parts->ip4;
            target->proxy_ipv6 = parts->ip6;
            target->proxy_port = parts->port;

            parts->host = 0;
            parts->ip4 = 0;
            parts->ip6 = 0;
            free_url_breakout( parts);
	}
        else
        {
            status->err_msg = strdup( "TEMP EMSG: Invalid proxy server string");
            rc = ERR_INVALID_DATA;
	}
    }

    if( rc == RC_NORMAL && url) if( *url)
    {
        parts = parse_url_string( url);
        if( !parts) rc = ERR_MALLOC_FAILED;
        else if( parts->status == URL_VALID)
        {
            target->ipv4 = parts->ip4;
            parts->ip4 = 0;

            target->ipv6 = parts->ip6;
            parts->ip6 = 0;

            if( target->conn_port == NO_PORT) target->conn_port = parts->port;

            if( !target->conn_host) empty = 1;
            else if( !*target->conn_host) empty = 1;
            if( empty)
            {
                target->conn_host = parts->host;
                parts->host = 0;
	    }

            if( !target->conn_uri) empty = 1;
            else if( !*target->conn_uri) empty = 1;
            if( empty)
            {
                if( parts->query)
                {
                    uri_len = 0;
                    if( parts->uri) uri_len += strlen( parts->uri);
                    uri_len += strlen( parts->query) + strlen( QUERY_DELIM) + 1;

                    target->conn_uri = (char *) malloc( uri_len);
                    if( !target->conn_uri) rc = ERR_MALLOC_FAILED;
                    else
                    {
                        *target->conn_uri = '\0';
                        if( parts->uri) strcat( target->conn_uri, parts->uri);
                        strcat( target->conn_uri, QUERY_DELIM);
                        strcat( target->conn_uri, parts->query);
		    }
		}
                else
                {
                    target->conn_uri = parts->uri;
                    parts->uri = 0;
		}
	    }

            free_url_breakout( parts);
	}
        else
        {
            status->err_msg = strdup( "TEMP EMSG: Invalid URL specified.");
            rc = ERR_INVALID_DATA;
	}
    }

    if( rc == RC_NORMAL)
    {
        if( !target->http_host) empty = 1;
        else if( !*target->http_host) empty = 1;
        else empty = 0;

        if( empty)
        {
            if( target->conn_host) target->http_host = strdup( target->conn_host);
            if( !target->http_host) rc = ERR_MALLOC_FAILED;
	}
    }

    if( rc == RC_NORMAL)
    {
        if( target->conn_port == NO_PORT) target->conn_port = DEFAULT_FETCH_PORT;
        status->last_state |= LS_FIND_CONNECTION;
    }

    return( rc);
}


/* --- */

int execute_fetch_plan( struct plan_data *plan)

{
    int rc = RC_NORMAL, seq, entry_state;
    struct exec_controls *runex = 0;
    struct output_options *out = 0;
    struct fetch_status *status = 0;

    /* --- */

    runex = plan->run;
    status = plan->status;
    out = plan->out;

    entry_state = status->last_state;

    for( seq = 1; rc == RC_NORMAL && seq <= runex->loop_count; seq++)
    {
        if( out->debug_level >= DEBUG_MEDIUM3) fprintf( out->info_out, "Loop start: %d of %d\n",
          seq, runex->loop_count);

        status->last_state = entry_state;

        clear_counters( &rc, plan);

/* ... need to lookup the proxy host too ... */

        lookup_connect_host( &rc, plan);

        connect_to_server( &rc, plan);

        send_request( &rc, plan);

        wait_for_reply( &rc, plan);

        pull_response( &rc, plan);

        display_output( &rc, plan, seq);

        if( runex->loop_pause > 0) usleep( runex->loop_pause);
    }

    /* --- */

    return( rc);
}

/* --- */

void clear_counters( int *rc, struct plan_data *plan)

{
    int sysrc;
    struct fetch_status *status = 0;
    struct ckpt_chain *walk = 0;
#ifdef USE_CLOCK_GETTIME
    struct timespec now;
    clockid_t clid = CLOCK_MONOTONIC;
#else
    struct timeval now;
#endif

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        status->clock_res = FRAC_RESOLUTION;
        status->response_len = 0;
        time( &status->wall_start);
    }

    if( *rc == RC_NORMAL)
    {
        for( walk = status->checkpoint; walk; walk = walk->next)
        {
            free_data_block( walk->detail);
            walk->detail = 0;
            walk->event = EVENT_BLANK;
            walk->clock.sec = 0;
            walk->clock.frac = 0;
	}

        if( !status->checkpoint)
        {
            status->err_msg = strdup( "TEMP EMSG: There should never be a null checkpoint, ugh.");
            *rc = ERR_UNSUPPORTED;
	}
        else
        {
#ifdef USE_CLOCK_GETTIME
            sysrc = clock_gettime( clid, &now);
#else
            sysrc = gettimeofday( &now, 0);
#endif
            if( !sysrc)
            {
                status->checkpoint->clock.sec = now.tv_sec;
#ifdef USE_CLOCK_GETTIME
                status->checkpoint->clock.frac = now.tv_nsec;
#else
                status->checkpoint->clock.frac = now.tv_usec;
#endif
                status->checkpoint->event = EVENT_START_FETCH;
	    }
            else
            {
                *rc = ERR_SYS_CALL;
                status->end_errno = errno;
#ifdef USE_CLOCK_GETTIME
                status->err_msg = strdup( "TEMP EMSG: Call to clock_gettime() failed.");
#else
                status->err_msg = strdup( "TEMP EMSG: Call to gettimeofday() failed.");
#endif
            }
	}
    }

    if( *rc == RC_NORMAL) status->last_state |= LS_CKPT_SETUP;

    return;
}

/* --- */

void lookup_connect_host( int *rc, struct plan_data *plan)

{
    int found = 0, sysrc;
    char display_ip[ INET6_ADDRSTRLEN + 1], *st = 0, *strc = 0;
    struct target_info *target = 0;
    struct fetch_status *status = 0;
    struct output_options *out = 0;
    struct addrinfo hints, *hostrecs = 0, *match4 = 0, *match6 = 0, *walk = 0;
    struct sockaddr_in *sock4 = 0;
    struct sockaddr_in6 *sock6 = 0;

    if( *rc == RC_NORMAL)
    {
        memset( display_ip, '\0', (sizeof display_ip));
        target = plan->target;
        status = plan->status;
        out = plan->out;
    }

    if( *rc == RC_NORMAL)
    {
        if( target->ipv6) if( *target->ipv6) found = 1;
        if( found)
        {
            found = 0;
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "Found an IPV6 address to use (%s)\n", target->ipv6);
            status->ip_type = IP_V6;
            sysrc = inet_pton( AF_INET6, target->ipv6, &status->sock6.sin6_addr);
            if( sysrc != 1)
            {
                status->end_errno = errno;
                status->err_msg = strdup( "TEMP EMSG: Lookup, IPv6 address failure.");
                *rc = ERR_SYS_CALL;
	    }
	}

        if( *rc == RC_NORMAL && target->ipv4) if( *target->ipv4) found = 1;
        if( found)
        {
            found = 0;
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "Found an IPV4 address to use (%s)\n", target->ipv4);
            status->ip_type = IP_V4;
            sysrc = inet_pton( AF_INET, target->ipv4, &status->sock4.sin_addr);
            if( sysrc != 1)
            {
                status->end_errno = errno;
                status->err_msg = strdup( "TEMP EMSG: Lookup, IPv4 address failure.");
                *rc = ERR_SYS_CALL;
	    }
	}

        if( *rc == RC_NORMAL && target->conn_host) if( *target->conn_host) found = 1;
        if( found)
        {
            found = 0;
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "Found a hostname to lookup (%s)\n", target->conn_host);

            hints.ai_flags = 0;
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            hints.ai_addrlen = 0;
            hints.ai_addr = 0;
            hints.ai_canonname = 0;
            hints.ai_next = 0;

            sysrc = getaddrinfo( target->conn_host, 0, &hints, &hostrecs);
            if( sysrc == EAI_NONAME)
            {
                if( out->debug_level >= DEBUG_MEDIUM1) fprintf( out->info_out,
                  "Can't lookup domain (%s)\n", target->conn_host);
                *rc = ERR_GETHOST_FAILED;
                status->err_msg = strdup( "TEMP EMSG: Lookup, no such hostname.");
                status->end_errno = sysrc;
                status->last_state |= LS_NO_DNS_CONNECT | LS_USE_GAI_ERRNO;
	    }
            else if( sysrc)
            {
                if( out->debug_level >= DEBUG_MEDIUM1) fprintf( out->err_out,
                  "Error: rc=%d (%s)\n", sysrc, gai_strerror( sysrc));
                status->err_msg = strdup( "TEMP EMSG: Lookup, hostname failure.");
                *rc = ERR_SYS_CALL;
                status->end_errno = sysrc;
                status->last_state |= LS_USE_GAI_ERRNO;
	    }
            else
            {
                match4 = match6 = 0;

                for( walk = hostrecs; walk; )
                {
                    if( !match4 && walk->ai_family == AF_INET) match4 = walk;
                    if( !match6 && walk->ai_family == AF_INET6) match6 = walk;
                    if( match4 && match6) walk = 0;
                    else walk = walk->ai_next;
		}

                if( match6)
                {
                    status->ip_type = IP_V6;
                    sock6 = (struct sockaddr_in6 *) match6->ai_addr;
                    status->sock6.sin6_addr = sock6->sin6_addr;
                    status->sock6.sin6_scope_id = sock6->sin6_scope_id;
                    status->sock6.sin6_port = htons( target->conn_port);
                    if( out->debug_level >= DEBUG_MEDIUM2)
                    {
                        st = display_ip;
                        strc = (char *) inet_ntop( AF_INET6, &sock6->sin6_addr, st, (sizeof display_ip));
                        if( !strc) strcat( display_ip, INVALID_IP);
                        fprintf( out->info_out, "Picked an IPV6 record (%s)\n", st);
		    }
		}
                else if( match4)
                {
                    status->ip_type = IP_V4;
                    sock4 = (struct sockaddr_in *) match4->ai_addr;
                    status->sock4.sin_addr = sock4->sin_addr;
                    status->sock4.sin_port = htons( target->conn_port);
                    if( out->debug_level >= DEBUG_MEDIUM2)
                    {
                        st = display_ip;
                        strc = (char *) inet_ntop( AF_INET, &sock4->sin_addr, st, (sizeof display_ip));
                        if( !strc) strcat( display_ip, INVALID_IP);
                        fprintf( out->info_out, "Picked an IPV4 record (%s)\n", st);
		    }
		}
                else
                {
                    status->err_msg = strdup( "TEMP EMSG: Lookup, no IPv4 or IPv6 records.");
                    *rc = ERR_GETHOST_FAILED;
                    if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
                      "No IPV4 or IPV6 host records found.\n");
		}

                freeaddrinfo( hostrecs);
	    }
	}

        if( *rc == RC_NORMAL && status->ip_type == IP_UNKNOWN)
        {
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "Oops! no idea what to lookup.\n");
            status->err_msg = strdup( "TEMP EMSG: Lookup, no records matched, this is odd...");
            *rc = ERR_UNSUPPORTED;
	}

        if( *rc == RC_NORMAL) *rc = capture_checkpoint( status, EVENT_DNS_LOOKUP);
        if( *rc == RC_NORMAL) status->last_state |= LS_CONNECT_LOOKUP;
    }

    return;
}

/* --- */

void connect_to_server( int *rc, struct plan_data *plan)

{
    int sock_type, sock, sysrc, salen;
#ifndef linux
    int flags;
#endif
    struct fetch_status *status = 0;
    struct output_options *out = 0;
    struct sockaddr *sa = 0;
    struct linger linger;
    struct pollfd netbox;
    struct exec_controls *runex = 0;
    struct sockaddr_in sock4;
    struct sockaddr_in6 sock6;

    memset( &sock4, '\0', sizeof sock4);
    memset( &sock6, '\0', sizeof sock6);

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        out = plan->out;
        runex = plan->run;
    }

    if( *rc == RC_NORMAL)
    {
        if( status->ip_type == IP_V4) sock_type = AF_INET;
        else sock_type = AF_INET6;

#ifdef linux
        sock = socket( sock_type, SOCK_STREAM | SOCK_NONBLOCK, 0);
#else
        sock = socket( sock_type, SOCK_STREAM, 0);
#endif
        if( sock == -1)
        {
            status->end_errno = errno;
            status->err_msg = strdup( "TEMP EMSG: Call to socket() failed.");
            *rc = ERR_SOCKET_FAILED;
	}
    }

    if( *rc == RC_NORMAL && runex->client_ip) if( *runex->client_ip)
    {
        if( is_ipv6_address( runex->client_ip))
        {
            sa = (struct sockaddr *) &sock6;
            sock6.sin6_family = AF_INET6;
            sock6.sin6_port = 0;
            sock6.sin6_flowinfo = 0;
            sock6.sin6_addr = in6addr_loopback;
            sock6.sin6_scope_id = SCOPE_LINK;
            salen = sizeof sock6;

            sysrc = inet_pton( AF_INET6, runex->client_ip, &sock6.sin6_addr);
            if( sysrc != 1)
            {
                status->err_msg = strdup( "TEMP EMSG: Call to inet_pton/6() failed.");
                *rc = ERR_INVALID_DATA;
	    }
	}
        else
        {
            sa = (struct sockaddr *) &sock4;
            sock4.sin_family = AF_INET;
            sock4.sin_port = 0;
            sock4.sin_addr.s_addr = 0;
            salen = sizeof sock4;

            sysrc = inet_pton( AF_INET, runex->client_ip, &sock4.sin_addr);
            if( sysrc != 1)
            {
                status->err_msg = strdup( "TEMP EMSG: Call to inet_pton/4() failed.");
                *rc = ERR_INVALID_DATA;
	    }
	}

        if( *rc == RC_NORMAL)
        {
            sysrc = bind( sock, sa, salen);
            if( sysrc == -1)
            {
printf( "DBG:: Bind-failed errno=%d\n", errno);
                status->end_errno = errno;
                status->err_msg = strdup( "TEMP EMSG: Call to bind() failed.");
                *rc = ERR_SYS_CALL;
	    }
	}
    }

    if( *rc == RC_NORMAL)
    {
        linger.l_onoff = 1;
        linger.l_linger = 300;
        sysrc = setsockopt( sock, SOL_SOCKET, SO_LINGER, &linger, sizeof linger);
        if( sysrc < 0)
        {
            status->end_errno = errno;
            status->err_msg = strdup( "TEMP EMSG: Setting linger didn't work.");
            *rc = ERR_SYS_CALL;
	}
    }

#ifndef linux
    if( *rc == RC_NORMAL)
    {
        flags = fcntl( sock, F_GETFL);
        if( flags >= 0) flags = fcntl( sock, F_SETFL, flags | FNONBLOCK);
        if( flags < 0)
        {
            status->end_errno = errno;
            status->err_msg = strdup( "TEMP EMSG: Can't make the socket non-blocking.");
            *rc = ERR_FCNTL_FAILED;
	}
    }
#endif

    if( *rc == RC_NORMAL)
    {
        if( sock_type == AF_INET)
        {
            sa = (struct sockaddr *) &status->sock4;
            salen = sizeof status->sock4;
	}
        else
        {
            sa = (struct sockaddr *) &status->sock6;
            salen = sizeof status->sock6;
	}

        sysrc = connect( sock, sa, salen);
        if( sysrc == 0 || errno == EINPROGRESS)
        {
            status->conn_sock = sock;
            netbox.fd = sock;
            netbox.events = POLL_EVENTS_WRITE;
            netbox.revents = 0;

            if( out->debug_level >= DEBUG_HIGH3) debug_timelog( "Pre connect-to-server");
            sysrc = poll( &netbox, 1, status->wait_timeout);
            if( out->debug_level >= DEBUG_HIGH3) debug_timelog( "Aft connect-to-server");
            if( sysrc == 0)
            {
                status->err_msg = strdup( "TEMP EMSG: Call to poll() timeout.");
                *rc = ERR_POLL_TIMEOUT;
	    }
            else if( sysrc == -1)
            {
                status->end_errno = errno;
                status->err_msg = strdup( "TEMP EMSG: Call to poll() failed.");
                *rc = ERR_SYS_CALL;
	    }
            else
            {
               if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
                 "Connect worked, sock=%d, type=%d\n", sock, status->ip_type);
               status->last_state |= LS_ESTAB_CONNECT;
            }
	}
        else
        {
            status->end_errno = errno;
            status->err_msg = strdup( "TEMP EMSG: Call to connect() failed.");
            *rc = ERR_CONN_FAILED;
	}

        if( *rc != RC_NORMAL) status->last_state |= LS_NO_CONNECTION;
        if( *rc == RC_NORMAL) *rc = capture_checkpoint( status, EVENT_CONNECT_SERVER);
    }
   

    return;
}

/* --- */

void send_request( int *rc, struct plan_data *plan)

{
    int sysrc;
    struct fetch_status *status;
    struct output_options *out;

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        out = plan->out;
    }

    if( *rc == RC_NORMAL)
    {
        sysrc = write( status->conn_sock, status->request, status->request_len);
        if( sysrc == status->request_len)
        {
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "Wrote the entire request successfully.\n");
            status->last_state |= LS_SENT_REQUEST;
	}
        else
        {
            status->end_errno = errno;
            status->last_state |= LS_NO_REQUEST;
            status->err_msg = strdup( "TEMP EMSG: Call to write() failed.");
            *rc = ERR_WRITE_FAILED;
	}

        if( *rc == RC_NORMAL) *rc = capture_checkpoint( status, EVENT_REQUEST_SENT);
    }

    return;
}

/* --- */

void wait_for_reply( int *rc, struct plan_data *plan)

{
    int sysrc;
    struct pollfd netbox;
    struct fetch_status *status;
    struct output_options *out;

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        out = plan->out;
    }

    if( *rc == RC_NORMAL)
    {
        netbox.fd = status->conn_sock;
        netbox.events = POLL_EVENTS_READ;
        netbox.revents = 0;

        if( out->debug_level >= DEBUG_HIGH3) debug_timelog( "Pre wait-for-reply");
        sysrc = poll( &netbox, 1, status->wait_timeout);
        if( out->debug_level >= DEBUG_HIGH3) debug_timelog( "Aft wait-for-reply");
        if( sysrc == 0)
        {
            status->err_msg = strdup( "TEMP EMSG: Call to poll() timeout.");
            *rc = ERR_POLL_TIMEOUT;
            status->last_state |= LS_NO_RESPONSE;
	}
        else if( sysrc == -1)
        {
            status->end_errno = errno;
            status->err_msg = strdup( "TEMP EMSG: Call to poll() failed.");
            *rc = ERR_SYS_CALL;
            status->last_state |= LS_NO_RESPONSE;
	}
        else
        {
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "Socket is ready for read() calls.\n");
            status->last_state |= LS_READ_READY;
            *rc = capture_checkpoint( status, EVENT_FIRST_RESPONSE);
	}
    }

    return;
}

/* --- */

void pull_response( int *rc, struct plan_data *plan)

{
    int done, sysrc, buffsize = READ_BUFF_SIZE, packlen = 0;
    char *buff = 0, *pos = 0;
    struct pollfd netbox;
    struct fetch_status *status;
    struct output_options *out;

    buff = (char *) malloc( buffsize);
    if( !buff) *rc = ERR_MALLOC_FAILED;

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        out = plan->out;
    }

    if( *rc == RC_NORMAL)
    {
        netbox.fd = status->conn_sock;
        netbox.events = POLL_EVENTS_READ;
        netbox.revents = 0;

        for( done = 0; !done && *rc == RC_NORMAL; )
        {
            if( out->debug_level >= DEBUG_HIGH3) debug_timelog( "Pre pull-response");
            sysrc = poll( &netbox, 1, status->wait_timeout);
            if( out->debug_level >= DEBUG_HIGH3) debug_timelog( "Aft pull-response");
            if( sysrc == 0)
            {
                status->err_msg = strdup( "TEMP EMSG: Call to poll() timeout.");
                *rc = ERR_POLL_TIMEOUT;
	    }
            else if( sysrc == -1)
            {
                status->end_errno = errno;
                status->err_msg = strdup( "TEMP EMSG: Call to poll() failed.");
                *rc = ERR_SYS_CALL;
	    }
            else
            {
                sysrc = packlen = read( status->conn_sock, buff, buffsize);
                if( sysrc == -1)
                {
                    if( errno == EINTR)
                    {
                        if( out->debug_level >= DEBUG_MEDIUM2) fprintf(
                          out->info_out, "Ignoring EINTR return from poll()\n");
		    }
                    else
                    {
                        status->end_errno = errno;
                        status->err_msg = strdup( "TEMP EMSG: Call to read() failed.");
                        *rc = ERR_READ_FAILED;
		    }
		}
                else if( !sysrc)
                {
                    status->last_state |= LS_GOT_RESPONSE;
                    done = 1;
                    *rc = capture_checkpoint( status, EVENT_READ_ALL_DATA);
		}
                else
                {
                    if( out->debug_level >= DEBUG_HIGH3)
                    {
                        fprintf( out->info_out, "Read %d bytes (", packlen);
                        for( pos = buff; pos < buff + packlen; pos++)
                          if( isprint( *pos) || *pos == '\n') fputc( *pos, out->info_out);
                          else fprintf( out->info_out, "<%02X>", *pos);
                        fprintf( out->info_out, ")\n");
		    }

                    status->response_len += sysrc;
                    *rc = capture_checkpoint( status, EVENT_READ_PACKET);
                    if( *rc == RC_NORMAL) *rc = add_data_block( status->lastcheck, sysrc, buff);
		}
	    }
	}

        if( status->conn_sock)
        {
            close( status->conn_sock);
            status->conn_sock = NO_SOCK;
	}

        if( *rc != RC_NORMAL) status->last_state |= LS_NO_PAYLOAD;
    }

    return;
}

/* --- */

struct chain_position *find_header_break( struct ckpt_chain *chain)

{
    char *fence = 0, *pos = 0, *last = 0;
    char *sng_pos = 0, sng_patt[ 3] = { '\n', '\n', '\0' };
    char *dbl_pos = 0, dbl_patt[ 5] = { '\r', '\n', '\r', '\n', '\0' };
    struct ckpt_chain *walk = 0;
    struct data_block *detail = 0;
    struct chain_position *result = 0;

    result = (struct chain_position *) malloc( sizeof *result);
    if( result)
    {
        result->position = 0;
        result->chain = 0;

        sng_pos = sng_patt;
        dbl_pos = dbl_patt;

        for( walk = chain; walk && !fence; walk = walk->next)
        {
            detail = walk->detail;
            if( detail)
            {
                pos = detail->data;
                last = pos + detail->len;

                for( ; pos <= last && !fence; pos++)
                {
                    if( *pos == *sng_pos) sng_pos++;
                    else sng_pos = sng_patt;

                    if( *pos == *dbl_pos) dbl_pos++;
                    else dbl_pos = dbl_patt;

                    if( !*sng_pos) fence = pos;
                    if( !*dbl_pos) fence = pos;

                    if( fence)
                    {
                        result->chain = walk;
                        result->position = fence;
		    }
		}
	    }
	}
    }

    if( !fence && result)
    {
        free( result);
        result = 0;
    }

    return( result);
}

/* --- */

void display_output( int *rc, struct plan_data *plan, int iter)

{
    int seq = 0, packlen = 0, in_head, disp_time_len = TIME_DISPLAY_SIZE, sysrc,
      dns_ms, conn_ms, resp_ms, close_ms, complete_ms;
    long top, now_sec = 0, now_sub = 0, prev_sec = 0, prev_sub = 0,
      diff_sec = 0, diff_sub = 0;
    float elap = 0.0, trans_rate = 0.0;
    char *pos = 0, *last_pos = 0;
    char disp_time[ TIME_DISPLAY_SIZE];
    struct fetch_status *status = 0;
    struct output_options *out = 0;
    struct ckpt_chain *walk = 0, *start = 0;
    struct summary_stats stats;
    struct data_block *detail = 0;
    struct display_settings *display = 0;
    struct chain_position *head_spot = 0;
    struct tm local_wall_start;

    stats.start_time = 0.0;
    stats.lookup_time = 0.0;
    stats.connect_time = 0.0;
    stats.request_time = 0.0;
    stats.response_time = 0.0;
    stats.complete_time = 0.0;

    status = plan->status;
    out = plan->out;
    display = plan->disp;
    top = (long) (1.0 / status->clock_res);

    if( *rc == RC_NORMAL)
    {
        head_spot = find_header_break( status->checkpoint);
        if( !head_spot)
        {
            status->err_msg = strdup( "TEMP EMSG: Can't find the end of the HTTP header.");
            *rc = ERR_UNSUPPORTED;
	}
    }

    if( *rc == RC_NORMAL && (display->show_head || display->show_data))
    {
        in_head = 1;
        walk = status->checkpoint;

        for( ; walk; walk = walk->next)
        {
            detail = walk->detail;
            if( detail) if( detail->len)
            {
                pos = detail->data;
                last_pos = pos + detail->len;
                for( ; pos < last_pos; pos++)
                {
                    if( in_head && display->show_head) fputc( *pos, stdout);
                    else if( !in_head && display->show_data) fputc( *pos, stdout);

                    if( pos == head_spot->position) in_head = 0;
		}
	    }
 	}
    }

    if( *rc == RC_NORMAL)
    {
        walk = status->checkpoint;

        for( ; walk; walk = walk->next)
        {
            if( start) elap = calc_time_difference( &start->clock, &walk->clock, status->clock_res);

            if( walk->event == EVENT_START_FETCH) start = walk;
            else if( walk->event == EVENT_DNS_LOOKUP) stats.lookup_time = elap;
            else if( walk->event == EVENT_CONNECT_SERVER) stats.connect_time = elap;
            else if( walk->event == EVENT_REQUEST_SENT) stats.request_time = elap;
            else if( walk->event == EVENT_FIRST_RESPONSE) stats.response_time = elap;
            else if( walk->event == EVENT_READ_ALL_DATA) stats.complete_time = elap;

            if( walk->event != EVENT_BLANK)
            {
                prev_sec = now_sec;
                prev_sub = now_sub;

                now_sec = walk->clock.sec; 
                now_sub = walk->clock.frac;

                diff_sec = now_sec - prev_sec;
                diff_sub = now_sub - prev_sub;
                if( diff_sub < 0)
                {
                    diff_sec--;
                    diff_sub += top;
                }

                if( walk->detail && display->show_packetime)
                {
                    packlen = walk->detail->len;
                    seq++;
#ifdef USE_CLOCK_GETTIME
                    fprintf( out->info_out, "%d. type: %d time: %ld.%09ld elap: %ld.%09ld data: %d elap: %.4f\n", 
                      seq, walk->event, walk->clock.sec, walk->clock.frac, diff_sec, diff_sub, packlen, elap);
#else
                    fprintf( out->info_out, "%d. type: %d time: %ld.%06ld elap: %ld.%06ld data: %d elap: %.4f\n", 
                      seq, walk->event, walk->clock.sec, walk->clock.frac, diff_sec, diff_sub, packlen, elap);
#endif
		}
	    }
	}
    }


/*

The output should look like this at some point.

-Date--TIme-     ----- Elapsed Time ---- Total Bytes
YrMnDyHrMnSe HRC   DNS  Conn 1stRD Close  Time  Xfer
------------ --- ----- ----- ----- ----- ----- -----
131015003945 200     1    79    95    84   259  7523
Average values:      1    79    95    84   259  7523

 */
    if( *rc == RC_NORMAL && display->show_timers)
    {
        if( stats.complete_time) trans_rate = (((float) status->response_len) / stats.complete_time) / 1024;
        else trans_rate = 0.0;

        fprintf( out->info_out, "Summary: dns: %.4f conn: %.4f sreq: %.4f resp: %.4f done: %.4f size: %ld K/sec: %.4f\n",
          stats.lookup_time, stats.connect_time, stats.request_time, stats.response_time,
          stats.complete_time, status->response_len, trans_rate);

        (void) localtime_r( &status->wall_start, &local_wall_start);
        disp_time[ 0] = '\0';
        sysrc = strftime( disp_time, disp_time_len, TIME_DISPLAY_FORMAT, &local_wall_start);
        if( !sysrc)
        {
            status->end_errno = errno;
            status->err_msg = strdup( "TEMP EMSG: Call to strftime() failed.");
            *rc = ERR_SYS_CALL;
	}

        if( *rc == RC_NORMAL)
        {
            if( iter == 1) fprintf( out->info_out, TIME_SUMMARY_HEADER);

            dns_ms = lroundf( stats.lookup_time * 1000.0);
            conn_ms = lroundf( (stats.connect_time - stats.lookup_time) * 1000.0);
            resp_ms = lroundf( (stats.response_time - stats.connect_time) * 1000.0);
            close_ms = lroundf( (stats.complete_time - stats.response_time) * 1000.0);
            complete_ms = lroundf( stats.complete_time * 1000.0);

            fprintf( out->info_out, "%s %3d %5d %5d %5d %5d %5d %6ld\n",
              disp_time, 999, dns_ms, conn_ms, resp_ms, close_ms, complete_ms, status->response_len);
	}
    }

    return;
}

/* --- */

int construct_request( struct plan_data *plan)

{
    int rc = RC_NORMAL, empty, ex_len;
    char *blank = "", *webhost = 0, *prefhost = 0, *agent = DEFAULT_FETCH_USER_AGENT,
      *uri = 0, *ex_headers = 0, *st = 0;
    struct target_info *target = 0;
    struct fetch_status *fetch = 0;
    struct output_options *out = 0;
    struct sub_list *subs = 0, *walk = 0;
    struct value_chain *chain = 0;

    /* --- */

    target = plan->target;
    fetch = plan->status;
    out = plan->out;

    /* --
     * Need to figure out how to handle the "Host:" header (or skip it)
     */
    webhost = target->http_host;
    if( !webhost) empty = 1;
    else if( !*webhost) empty = 1;
    else empty = 0;

    if( empty) webhost = target->conn_host;
    if( !webhost) empty = 1;
    else if( !*webhost) empty = 1;
    else empty = 0;

    if( empty)
    {
        webhost = blank;
        prefhost = HTTP_HEADER_XPREF;
    }
    else prefhost = blank;

    /* --- */

    uri = target->conn_uri;
    if( !uri) empty = 1;
    else if( !*uri) empty = 1;
    else empty = 0;

    if( empty) uri = ROOT_URI;

    /* --- */

    ex_len = 0;

    chain = target->extra_headers;
    for( ; chain; chain = chain->next)
    {
        st = (char *) chain->parsed;
#ifdef NO_CR
        if( *st) ex_len += strlen( st) + strlen( NO_CR_HTTP_EOL);
#else
        if( *st) ex_len += strlen( st) + strlen( HTTP_EOL);
#endif
    }

    if( ex_len)
    {
        ex_headers = (char *) malloc( ex_len + 1);
        if( !ex_headers) rc = ERR_MALLOC_FAILED;
        else
        {
            *ex_headers = '\0';

            chain = target->extra_headers;
            for( ; chain; chain = chain->next)
            {
                st = (char *) chain->parsed;
                if( *st)
                {
                    strcat( ex_headers, (char *) chain->parsed);
                    strcat( ex_headers, HTTP_EOL);
		}
	    }
	}
    }

    if( rc == RC_NORMAL) if( !ex_headers) ex_headers = blank;

    /* --- */

    if( rc == RC_NORMAL)
    {
        walk = subs = (struct sub_list *) malloc( sizeof *subs);
        if( !walk) rc = ERR_MALLOC_FAILED;
        else
        {
            walk->from = PATT_URI;
            walk->to = uri;
            walk->next = (struct sub_list *) malloc( sizeof *walk);
            if( !walk->next) rc = ERR_MALLOC_FAILED;
            else walk = walk->next;
	}
    }

    if( rc == RC_NORMAL)
    {
        walk->from = PATT_EXTRA_HEADERS;
        walk->to = ex_headers;
        walk->next = (struct sub_list *) malloc( sizeof *walk);
        if( !walk->next) rc = ERR_MALLOC_FAILED;
        else walk = walk->next;
    }

    if( rc == RC_NORMAL)
    {
        walk->from = PATT_HOST_COMMENT;
        walk->to = prefhost;
        walk->next = (struct sub_list *) malloc( sizeof *walk);
        if( !walk->next) rc = ERR_MALLOC_FAILED;
        else walk = walk->next;
    }

    if( rc == RC_NORMAL)
    {
        walk->from = PATT_HOST_NAME;
        walk->to = webhost;
        walk->next = (struct sub_list *) malloc( sizeof *walk);
        if( !walk->next) rc = ERR_MALLOC_FAILED;
        else walk = walk->next;
    }

    if( rc == RC_NORMAL)
    {
        walk->from = PATT_USER_AGENT;
        walk->to = agent;
        walk->next = 0;
    }

#ifdef NO_CR
    if( rc == RC_NORMAL) fetch->request = gsub_string( &rc, NO_CR_FETCH_REQUEST_TEMPLATE, subs);
#else
    if( rc == RC_NORMAL) fetch->request = gsub_string( &rc, FETCH_REQUEST_TEMPLATE, subs);
#endif

    if( rc == RC_NORMAL) fetch->request_len = strlen( fetch->request);

    if( rc == RC_NORMAL) if( out->debug_level >= DEBUG_HIGH2) fprintf( out->info_out,
      "- - - HTTP request - - -\n%s\n- - -\n", fetch->request);

    if( rc == RC_NORMAL) fetch->last_state |= LS_GEN_REQUEST;

    /* --- */

    return( rc);
}

/* --- */

struct plan_data *allocate_plan_data()

{
    int error = 0;
    struct plan_data *plan = 0;
    struct target_info *target = 0;
    struct display_settings *disp = 0;
    struct exec_controls *runex = 0;
    struct output_options *out = 0;
    struct fetch_status *status = 0;
    struct ckpt_chain *checkpoint = 0;

    /* --- */

    plan = (struct plan_data *) malloc( sizeof *plan);
    if( !plan) error = 1;

    if( !error)
    {
        target = (struct target_info *) malloc( sizeof *target);
        if( !target) error = 1;
    }

    if( !error)
    {
        disp = (struct display_settings *) malloc( sizeof *disp);
        if( !disp) error = 1;
    }

    if( !error)
    {
        runex = (struct exec_controls *) malloc( sizeof *runex);
        if( !runex) error = 1;
    }

    if( !error)
    {
        out = (struct output_options *) malloc( sizeof *out);
        if( !out) error = 1;
    }

    if( !error)
    {
        status = (struct fetch_status *) malloc( sizeof *status);
        if( !status) error = 1;
    }

    if( !error)
    {
        checkpoint = (struct ckpt_chain *) malloc( sizeof *checkpoint);
        if( !checkpoint) error = 1;
    }

    if( error)
    {
        if( plan) free( plan);
        if( target) free( target);
        if( disp) free( disp);
        if( runex) free( runex);
        if( out) free( out);
        if( status) free( status);
        if( checkpoint) free( checkpoint);
        plan = 0;
    }
    else
    {
        checkpoint->event = EVENT_START_FETCH;
        checkpoint->clock.sec = 0;
        checkpoint->clock.frac = 0;
        checkpoint->detail = 0;
        checkpoint->next = 0;

        plan->target = target;
        plan->disp = disp;
        plan->run = runex;
        plan->out = out;
        plan->status = status;

        target->http_host = 0;
        target->conn_host = 0;
        target->conn_uri = 0;
        target->conn_url = 0;
        target->ipv4 = 0;
        target->ipv6 = 0;
        target->auth_user = 0;
        target->auth_passwd = 0;
        target->proxy_url = 0;
        target->proxy_host = 0;
        target->proxy_ipv4 = 0;
        target->proxy_ipv6 = 0;
        target->extra_headers = 0;
        target->conn_port = NO_PORT;
        target->proxy_port = NO_PORT;
        target->conn_pthru = 0;

        out->out_html = 0;
        out->debug_level = 0;
        out->info_out = stdout;
        out->err_out = stderr;

        disp->show_head = 0;
        disp->show_data = 0;
        disp->show_timers = 0;
        disp->show_timerheaders = 0;
        disp->show_packetime = 0;
        disp->show_help = 0;
        disp->show_complete = 0;
        disp->show_number = 0;

        runex->loop_count = 0;
        runex->loop_pause = 0;
        runex->conn_timeout = 0;
        runex->client_ip = 0;

        status->response_len = 0;
        status->ip_type = IP_UNKNOWN;
        status->conn_sock = NO_SOCK;
        status->request_len = 0;
        status->wait_timeout = DEF_WAIT_TIMEOUT;
        status->last_state = LS_START;
        status->end_errno = 0;
        status->err_msg = 0;
        status->request = 0;
        status->clock_res = 0.0;

        status->sock4.sin_family = AF_INET;
        status->sock4.sin_port = htons( DEFAULT_FETCH_PORT);
        status->sock4.sin_addr.s_addr = 0;
        memset( status->sock4.sin_zero, '\0', sizeof *status->sock4.sin_zero);

        status->sock6.sin6_family = AF_INET6;
        status->sock6.sin6_port = htons( DEFAULT_FETCH_PORT);
        status->sock6.sin6_flowinfo = 0;
        status->sock6.sin6_addr = in6addr_loopback;
        status->sock6.sin6_scope_id = SCOPE_LINK;

        status->checkpoint = checkpoint;
        status->lastcheck = checkpoint;
    }

    return( plan);
}

/* --- */

struct plan_data *figure_out_plan( int *returncode, int narg, char **opts)
{
    int rc = RC_NORMAL, errlen, in_val, nop_head = 0, nop_data = 0, nop_comp = 0;
    char *sep = 0, *cgi_data = 0, *unrecognized = 0, *st = 0;
    struct option_set opset[] = {
      { OP_HEADER,       OP_TYPE_FLAG, OP_FL_BLANK,   FL_HEADER,         0, DEF_HEADER,       0, 0 },
      { OP_HEADER,       OP_TYPE_FLAG, OP_FL_BLANK,   FL_HEADER_2,       0, DEF_HEADER,       0, 0 },
      { OP_OUTPUT,       OP_TYPE_FLAG, OP_FL_BLANK,   FL_OUTPUT,         0, DEF_OUTPUT,       0, 0 },
      { OP_OUTPUT,       OP_TYPE_FLAG, OP_FL_BLANK,   FL_OUTPUT_2,       0, DEF_OUTPUT,       0, 0 },
      { OP_URI,          OP_TYPE_CHAR, OP_FL_BLANK,   FL_URI,            0, DEF_URI,          0, 0 },
      { OP_CONNHOST,     OP_TYPE_CHAR, OP_FL_BLANK,   FL_CONNHOST,       0, DEF_CONNHOST,     0, 0 },
      { OP_WEBSERVER,    OP_TYPE_CHAR, OP_FL_BLANK,   FL_WEBSERVER,      0, DEF_WEBSERVER,    0, 0 },
      { OP_URL,          OP_TYPE_CHAR, OP_FL_BLANK,   FL_URL,            0, DEF_URL,          0, 0 },
      { OP_PORT,         OP_TYPE_INT,  OP_FL_BLANK,   FL_PORT,           0, DEF_PORT,         0, 0 },
      { OP_TIMERS,       OP_TYPE_FLAG, OP_FL_BLANK,   FL_TIMERS,         0, DEF_TIMERS,       0, 0 },
      { OP_TIMERS,       OP_TYPE_FLAG, OP_FL_BLANK,   FL_TIMERS_2,       0, DEF_TIMERS,       0, 0 },
      { OP_DATA,         OP_TYPE_FLAG, OP_FL_BLANK,   FL_DATA,           0, DEF_DATA,         0, 0 },
      { OP_LOOP,         OP_TYPE_INT,  OP_FL_BLANK,   FL_LOOP,           0, DEF_LOOP,         0, 0 },
      { OP_HTML,         OP_TYPE_FLAG, OP_FL_BLANK,   FL_HTML,           0, DEF_HTML,         0, 0 },
      { OP_NUMBER,       OP_TYPE_FLAG, OP_FL_BLANK,   FL_NUMBER,         0, DEF_NUMBER,       0, 0 },
      { OP_PAUSE,        OP_TYPE_INT,  OP_FL_BLANK,   FL_PAUSE,          0, DEF_PAUSE,        0, 0 },
      { OP_XHEADER,      OP_TYPE_CHAR, OP_FL_REPEATS, FL_XHEADER,        0, DEF_XHEADER,      0, 0 },
      { OP_USESTDERR,    OP_TYPE_FLAG, OP_FL_BLANK,   FL_USESTDERR,      0, DEF_USESTDERR,    0, 0 },
      { OP_AUTH,         OP_TYPE_CHAR, OP_FL_BLANK,   FL_AUTH,           0, DEF_AUTH,         0, 0 },
      { OP_TIMERHEADERS, OP_TYPE_FLAG, OP_FL_BLANK,   FL_TIMERHEADERS,   0, DEF_TIMERHEADERS, 0, 0 },
      { OP_TIMERHEADERS, OP_TYPE_FLAG, OP_FL_BLANK,   FL_TIMERHEADERS_2, 0, DEF_TIMERHEADERS, 0, 0 },
      { OP_TIMEOUT,      OP_TYPE_INT,  OP_FL_BLANK,   FL_TIMEOUT,        0, DEF_TIMEOUT,      0, 0 },
      { OP_PROXY,        OP_TYPE_CHAR, OP_FL_BLANK,   FL_PROXY,          0, DEF_PROXY,        0, 0 },
      { OP_PACKETIME,    OP_TYPE_FLAG, OP_FL_BLANK,   FL_PACKETIME,      0, DEF_PACKETIME,    0, 0 },
      { OP_CLIENTIP,     OP_TYPE_CHAR, OP_FL_BLANK,   FL_CLIENTIP,       0, DEF_CLIENTIP,     0, 0 },
      { OP_PTHRU,        OP_TYPE_FLAG, OP_FL_BLANK,   FL_PTHRU,          0, DEF_PTHRU,        0, 0 },
      { OP_DEBUG,        OP_TYPE_INT,  OP_FL_BLANK,   FL_DEBUG,          0, DEF_DEBUG,        0, 0 },
    };
    struct option_set *co = 0;
    struct word_chain *extra_opts = 0, *walk = 0;
    int nflags = (sizeof opset) / (sizeof opset[0]);
    struct plan_data *plan = 0;
    struct target_info *target = 0;
    struct display_settings *display = 0;
    struct exec_controls *runex = 0;
    struct output_options *out = 0;
    struct fetch_status *status = 0;

    /* --- */

    plan = allocate_plan_data();
    if( !plan) rc = ERR_MALLOC_FAILED;
    {
        target = plan->target;
        display = plan->disp;
        runex = plan->run;
        out = plan->out;
        status = plan->status;
    }

    /* --- 
     * Get the details of the request, either from the command line or from CGI data,
     *   parse the request details using the recognized options/flags.
     */
   
    if( rc == RC_NORMAL)
    {
        if( called_as_cgi())
        {
/*            is_cgi = 1; */
            out->info_out = stdout;
            out->err_out = stdout;
            fprintf( out->info_out, "%s\n", HTML_RESP_HEADER);
            cgi_data = get_cgi_data( &rc);
            if( rc == RC_NORMAL) extra_opts = parse_cgi_options( &rc, opset, nflags, cgi_data);
	}
        else
        {
            if( narg < 2) display->show_help = 1;

            extra_opts = parse_command_options( &rc, opset, nflags, narg, opts);
	}

        if( rc == RC_NORMAL) status->last_state |= LS_PARSED_OPTIONS;

        /* Need to pull this one up to enable debug logging (other options are parsed later) */

        if(( co = cond_get_matching_option( &rc, OP_DEBUG, opset, nflags))) out->debug_level = *((int *) co->parsed);

        if(( co = cond_get_matching_option( &rc, OP_USESTDERR, opset, nflags)))
        {
            in_val = *((int *) co->parsed);
            if( in_val) out->info_out = stderr;
            SHOW_OPT_IF_DEBUG( "use-stderr")
            if( out->debug_level >= DEBUG_HIGH1) fprintf( out->info_out, "Opt #%d, debug '%d'\n",
              co->opt_num, *((int *) co->parsed));
        }

    }


    /* ---
     * If there was extraneous junk in the request data, construct one character string from all
     *   the pieces.
     */

    if( rc == RC_NORMAL)
    {
        for( walk = extra_opts, errlen = 0; walk; walk = walk->next)
          if( walk->opt) if( *walk->opt) errlen += strlen( walk->opt) + 1;

        if( errlen)
        {
            status->err_msg = strdup( "TEMP EMSG: Unrecognized options on command line.");
            rc = ERR_SYNTAX;

            unrecognized = (char *) malloc( errlen + 1);
            if( !unrecognized) rc = ERR_MALLOC_FAILED;
            else
            {
                *unrecognized = '\0';
                for( walk = extra_opts; walk; )
                {
                    if( walk->opt) strcat( unrecognized, walk->opt);
                    walk = walk->next;
                    if( walk) if( walk->opt) strcat( unrecognized, " ");
		}

                errlen += strlen( ERRMSG_UNRECOG_OPTIONS);
                status->err_msg = (char *) malloc( errlen);
                if( !status->err_msg) rc = ERR_MALLOC_FAILED;
                else snprintf( status->err_msg, errlen, ERRMSG_UNRECOG_OPTIONS, unrecognized);

                free( unrecognized);
	    }
	}

        /* ---
         * This chunk just displays a dump of the options supported and how the parsed data overlays onto
         *   that model.  It's only shown if the debug level requested is high.
         */

        if( out->debug_level >= DEBUG_HIGH1)
        {
            fprintf( out->info_out, "rc=%d extra(", rc);
            sep = "";
            for( walk = extra_opts; walk; )
            {
                fprintf( out->info_out, "%s%s", sep, walk->opt);
                walk = walk->next;
                if( walk) sep = " ";
                else sep = "";
            }
            fprintf( out->info_out, ")\n");

            print_option_settings( out->info_out, nflags, opset);
        }
    }

    /* ---
     */

    if(( co = cond_get_matching_option( &rc, OP_HEADER, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "header")
        display->show_head = *((int *) co->parsed);
        nop_head = co->opt_num;
    }

    if(( co = cond_get_matching_option( &rc, OP_OUTPUT, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "all-output")
        display->show_complete = *((int *) co->parsed);
        nop_comp = co->opt_num;
    }

    if(( co = cond_get_matching_option( &rc, OP_URI, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "URI")
        target->conn_uri = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_CONNHOST, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "connhost")
        target->conn_host = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_WEBSERVER, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "webhost")
        target->http_host = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_URL, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "URL")
        target->conn_url = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_PORT, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "port")
        target->conn_port = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_TIMERS, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "timers")
        display->show_timers = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_DATA, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "data")
        display->show_data = *((int *) co->parsed);
        nop_data = co->opt_num;
    }

    if(( co = cond_get_matching_option( &rc, OP_LOOP, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "loop")
        runex->loop_count = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_HTML, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "HTML")
        out->out_html = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_NUMBER, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "show-number")
        display->show_number = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_PAUSE, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "pause")
        runex->loop_pause = *((int *) co->parsed) * 1000;
    }
/*
    if(( co = cond_get_matching_option( &rc, OP_USESTDERR, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "use-stderr")
        in_val = *((int *) co->parsed);
        if( in_val) out->info_out = stderr;
    }
 */
    if(( co = cond_get_matching_option( &rc, OP_TIMERHEADERS, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "timerheaders")
        display->show_timerheaders = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_TIMEOUT, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "timeout")
        runex->conn_timeout = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_PROXY, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "proxy")
        target->proxy_url = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_PACKETIME, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "show-packet")
        display->show_packetime = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_CLIENTIP, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "client-ip")
        runex->client_ip = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_PTHRU, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "passthru")
        target->conn_pthru = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_XHEADER, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "extra-header")
        target->extra_headers = (struct value_chain *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_AUTH, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( "auth")
        target->auth_user = strdup( (char *) co->parsed);
        st = index( target->auth_user, COLON_CH);
        if( st)
        {
            *st = '\0';
            target->auth_passwd = strdup( st + 1);
        }
        else
        {
            target->auth_passwd = (char *) malloc( 1);
            if( target->auth_passwd) *target->auth_passwd = '\0';
        }
        if( !target->auth_passwd) rc = ERR_MALLOC_FAILED;
    }

    /* ---
     * If the options controlling which parts of the response conflict, then
     *   check where each was given on the command line to decide which
     *   overrides.
     */

    if( !display->show_complete && (display->show_head || display->show_data))
    {
        if( nop_comp > nop_head && nop_comp > nop_data)
        {
            display->show_head = 0;
            display->show_data = 0;
	}
    }

    else if( display->show_complete && (!display->show_head || !display->show_data))
    {
        if( nop_comp > nop_head && nop_comp > nop_data)
        {
            display->show_head = 1;
            display->show_data = 1;
	}
        else display->show_complete = 0;
    }

    /* --- */

    if( runex->loop_count < 1) runex->loop_count = 1;

    /* --- */

    return( plan);
}

/* --- */

int capture_checkpoint( struct fetch_status *status, int event_type)

{
    int rc = RC_NORMAL, sysrc;
    struct ckpt_chain *anchor= 0, *curr = 0;
#ifdef USE_CLOCK_GETTIME
    struct timespec now;
    clockid_t clid = CLOCK_MONOTONIC;
#else
    struct timeval now;
#endif

    anchor = status->lastcheck;
    if( !anchor)
    {
        status->err_msg = strdup( "TEMP EMSG: Weird, there's no anchor...");
        rc = ERR_UNSUPPORTED;
    }
    else
    {
        curr = anchor->next;
        if( !curr)
        {
            curr = (struct ckpt_chain *) malloc( sizeof *curr);
            if( !curr) rc = ERR_MALLOC_FAILED;
            else
            {
                anchor->next = curr;
                curr->next = 0;
                curr->clock.sec = 0;
                curr->clock.frac = 0;
                curr->detail = 0;
	    }
	}

        if( curr)
        {
            status->lastcheck = curr;
            curr->event = event_type;
            free_data_block( curr->detail);
            curr->detail = 0;
#ifdef USE_CLOCK_GETTIME
            sysrc = clock_gettime( clid, &now);
#else
            sysrc = gettimeofday( &now, 0);
#endif
            if( !sysrc)
            {
                curr->clock.sec = now.tv_sec;
#ifdef USE_CLOCK_GETTIME
                curr->clock.frac = now.tv_nsec;
#else
                curr->clock.frac = now.tv_usec;
#endif
	    }
            else
            {
                status->end_errno = errno;
#ifdef USE_CLOCK_GETTIME
                status->err_msg = strdup( "TEMP: Call to clock_gettime() failed in capture_checkpoint()");
#else
                status->err_msg = strdup( "TEMP: Call to clock_gettime() failed in gettimeofday()");
#endif
                rc = ERR_SYS_CALL;
	    }
	}
    }

    return( rc);
}

/* --- */

int main( int narg, char **opts)

{
    int rc = RC_NORMAL;
    char *emsg = 0;
    struct plan_data *plan = 0;
    struct target_info *target = 0;
    struct display_settings *disp = 0;
    struct exec_controls *runex = 0;
    struct output_options *out = 0;
    struct fetch_status *fetch = 0;
    struct value_chain *chain = 0;

    /* --- */

    plan = figure_out_plan( &rc, narg, opts);

    if( rc == RC_NORMAL)
    {
        target = plan->target;
        disp = plan->disp;
        runex = plan->run;
        out = plan->out;
        fetch = plan->status;
    }

    if( rc == RC_NORMAL) rc = find_connection( plan);

/* ... the request build has to consider the "proxy" settings and "connect thru" as well ... */
    if( rc == RC_NORMAL) rc = construct_request( plan);

    if( rc == RC_NORMAL) if( plan->out->debug_level >= DEBUG_HIGH2)
    {
        fprintf( out->info_out, "- - Target:\n");
        fprintf( out->info_out, "- - - - ipv4: (%s)\n", SPSP( target->ipv4));
        fprintf( out->info_out, "- - - - ipv6: (%s)\n", SPSP( target->ipv6));
        fprintf( out->info_out, "- - - - web-host: (%s)\n", SPSP( target->http_host));
        fprintf( out->info_out, "- - - - conn-host: (%s)\n", SPSP( target->conn_host));
        fprintf( out->info_out, "- - - - conn-uri: (%s)\n", SPSP( target->conn_uri));
        fprintf( out->info_out, "- - - - conn-url: (%s)\n", SPSP( target->conn_url));
        fprintf( out->info_out, "- - - - auth-user: (%s)\n", SPSP( target->auth_user));
        fprintf( out->info_out, "- - - - auth-password: (%s)\n", SPSP( target->auth_passwd));
        fprintf( out->info_out, "- - - - proxy-url: (%s)\n", SPSP( target->proxy_url));
        fprintf( out->info_out, "- - - - proxy-host: (%s)\n", SPSP( target->proxy_host));
        fprintf( out->info_out, "- - - - proxy-ipv4: (%s)\n", SPSP( target->proxy_ipv4));
        fprintf( out->info_out, "- - - - proxy-ipv6: (%s)\n", SPSP( target->proxy_ipv6));
        for( chain = target->extra_headers; chain; chain = chain->next)
          fprintf( out->info_out, "- - - - extra-headers: (%s)\n", SPSP( (char *) chain->parsed));
        fprintf( out->info_out, "- - - - conn-port: (%d)\n", target->conn_port);
        fprintf( out->info_out, "- - - - proxy-port: (%d)\n", target->proxy_port);
        fprintf( out->info_out, "- - - - conn-thru: (%d)\n", target->conn_pthru);

        fprintf( out->info_out, "\n- - Display:\n");
        fprintf( out->info_out, "- - - - show-header: %d\n", disp->show_head);
        fprintf( out->info_out, "- - - - show-data: %d\n", disp->show_data);
        fprintf( out->info_out, "- - - - show-timers: %d\n", disp->show_timers);
        fprintf( out->info_out, "- - - - show-timerheaders: %d\n", disp->show_timerheaders);
        fprintf( out->info_out, "- - - - show-packetime: %d\n", disp->show_packetime);
        fprintf( out->info_out, "- - - - show-help: %d\n", disp->show_help);
        fprintf( out->info_out, "- - - - show-all-results: %d\n", disp->show_complete);
        fprintf( out->info_out, "- - - - show-number: %d\n", disp->show_number);

        fprintf( out->info_out, "\n- - Output:\n");
        fprintf( out->info_out, "- - - - out-html: %d\n", out->out_html);
        fprintf( out->info_out, "- - - - debug-level: %d\n", out->debug_level);
        fprintf( out->info_out, "- - - - info-out: %d\n", fileno( out->info_out));
        fprintf( out->info_out, "- - - - err-out: %d\n", fileno( out->err_out));

        fprintf( out->info_out, "\n- - RunEx:\n");
        fprintf( out->info_out, "- - - - loop-count: %d\n", runex->loop_count);
        fprintf( out->info_out, "- - - - loop-pause: %d\n", runex->loop_pause);
        fprintf( out->info_out, "- - - - conn-timeout: %d\n", runex->conn_timeout);
        fprintf( out->info_out, "- - - - client-ip: (%s)\n", SPSP( runex->client_ip));

        fprintf( out->info_out, "\n- - FetchStatus:\n");
        fprintf( out->info_out, "- - - - last-state: %d\n", fetch->last_state);
        fprintf( out->info_out, "- - - - errmsg: (%s)\n", SPSP( fetch->err_msg));
        fprintf( out->info_out, "- - - - request: (%s)\n", SPSP( fetch->request));
    }

    if( rc == RC_NORMAL) rc = execute_fetch_plan( plan);

    /* --- */

    if( rc != RC_NORMAL)
    {
        emsg = fetch->err_msg;
        if( !emsg) emsg = UNDEFINED_ERROR;
        else if( !*emsg) emsg = UNDEFINED_ERROR;

        fprintf( out->err_out, "Error(%d/%06x): %s %s\n", rc, fetch->last_state, cli_strerror( rc), emsg);
    }

    if( out) if( out->debug_level >= DEBUG_MEDIUM1) fprintf( out->info_out,
      "End state: %06X, End err: %d\n", fetch->last_state, fetch->end_errno);

    /* --- */

    exit( rc);
}
