/*
#define SHOW_DEBUG_STATS_CALCS
#define SHOW_GORY_XFRATE_CALC_DETAILS
 */

/*
 * Bugs:
 * ---
 *
 * Features:
 * ---
 * - Should have an option to follow redirects
 *
 * - Options: auth, proxy, connthru
 *
 * Notes:
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
#include <net/if.h>

#include "http-fetch.h"
#include "cli-sub.h"
#include "err_ref.h"

/* --- */

void debug_timelog( FILE *out, char *prefix, char *tag)

{
    static int seq = 0;
    long diff_sec, diff_sub, top = 1000000, now_sec, now_sub;
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

    fprintf( out, "%sdbg:: %3d. %11ld.%06ld %11ld.%06ld (%s)\n", prefix, seq, now_sec, now_sub, diff_sec, diff_sub, tag);

    prev.tv_sec = now.tv_sec;
    prev.tv_usec = now.tv_usec;

    return;
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
            else
            {
                (void) memcpy( packet->data, buff, len);
                checkpoint->detail = packet;
	    }
	}

        if( rc != RC_NORMAL) if( packet) free( packet);
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
            status->err_msg = strdup( EMSG_INVALID_PROXY);
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
                        *target->conn_uri = EOS_CH;
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
            status->err_msg = strdup( EMSG_INVALID_URL);
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
    struct display_settings *disp = 0;

    /* --- */

    runex = plan->run;
    status = plan->status;
    out = plan->out;
    disp = plan->disp;

    entry_state = status->last_state;

    for( seq = 1; rc == RC_NORMAL && seq <= runex->loop_count; seq++)
    {
        if( out->debug_level >= DEBUG_MEDIUM3) fprintf( out->info_out, "%sLoop start: %d of %d\n",
          disp->line_pref, seq, runex->loop_count);

        status->last_state = entry_state;

        clear_counters( &rc, plan);

/* ... need to lookup the proxy host too ... */

        lookup_connect_host( &rc, plan);

        connect_to_server( &rc, plan);

        send_request( &rc, plan);

        wait_for_reply( &rc, plan);

        pull_response( &rc, plan);

        parse_payload( &rc, plan);

        stats_from_packets( &rc, plan, seq);

        display_output( &rc, plan, seq);

        if( runex->loop_pause > 0 && seq < runex->loop_count) usleep( runex->loop_pause);
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
            status->err_msg = strdup( EMSG_UNRECOV_NULL_CHECKPOINT);
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
                status->err_msg = sys_call_fail_msg( "clock_gettime");
#else
                status->err_msg = sys_call_fail_msg( "gettimeofday");
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
    struct exec_controls *runex = 0;
    struct addrinfo hints, *hostrecs = 0, *match4 = 0, *match6 = 0, *walk = 0;
    struct sockaddr_in *sock4 = 0;
    struct sockaddr_in6 *sock6 = 0;
    struct display_settings *disp = 0;

    if( *rc == RC_NORMAL)
    {
        memset( display_ip, EOS_CH, (sizeof display_ip));
        target = plan->target;
        status = plan->status;
        out = plan->out;
        runex = plan->run;
        disp = plan->disp;
    }

    if( *rc == RC_NORMAL)
    {
        if( target->ipv6) if( *target->ipv6) found = 1;
        if( found)
        {
            found = 0;
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "%sFound an IPV6 address to use (%s)\n", disp->line_pref, target->ipv6);
            status->ip_type = IP_V6;
            sysrc = inet_pton( AF_INET6, target->ipv6, &status->sock6.sin6_addr);
            if( sysrc != 1)
            {
                status->end_errno = errno;
                status->err_msg = strdup( EMSG_BAD_IPV6_ADDRESS);
                *rc = ERR_SYS_CALL;
	    }
	}
    }

    if( *rc == RC_NORMAL)
    {
        if( target->ipv4) if( *target->ipv4) found = 1;
        if( found)
        {
            found = 0;
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "%sFound an IPV4 address to use (%s)\n", disp->line_pref, target->ipv4);
            status->ip_type = IP_V4;
            sysrc = inet_pton( AF_INET, target->ipv4, &status->sock4.sin_addr);
            if( sysrc != 1)
            {
                status->end_errno = errno;
                status->err_msg = strdup( EMSG_BAD_IPV4_ADDRESS);
                *rc = ERR_SYS_CALL;
	    }
	}
    }

    if( *rc == RC_NORMAL)
    {
        if( target->conn_host) if( *target->conn_host) found = 1;
        if( found)
        {
            found = 0;
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "%sFound a hostname to lookup (%s)\n", disp->line_pref, target->conn_host);

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
                  "%sCan't lookup domain (%s)\n", disp->line_pref, target->conn_host);
                *rc = ERR_GETHOST_FAILED;
                status->err_msg = errmsg_with_string( EMSG_TEMP_LOOKUP_NO_SUCH_HOST, target->conn_host);
                status->end_errno = sysrc;
                status->last_state |= LS_NO_DNS_CONNECT | LS_USE_GAI_ERRNO;
	    }
            else if( sysrc)
            {
                if( out->debug_level >= DEBUG_MEDIUM1) fprintf( out->err_out,
                  "%sError: rc=%d (%s)\n", disp->line_pref, sysrc, gai_strerror( sysrc));
                status->err_msg = errmsg_with_string( EMSG_TEMP_LOOKUP_BAD_RC, gai_strerror( sysrc));
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

                /* If we've been asked to bind the socket to an IPV6 address
                 * then we have to pick an IPV6 server address.  The same goes
                 * for IPV4 binds as well.
                 */
                if( runex->client_ip_type == IP_V6) match4 = 0;
                else if( runex->client_ip_type == IP_V4) match6 = 0;
                else if( target->pref_protocol == IP_V6) match4 = 0;
                else if( target->pref_protocol == IP_V4) match6 = 0;

                if( match4)
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
                        fprintf( out->info_out, "%sPicked an IPV4 record (%s)\n", disp->line_pref, st);
		    }
		}
                else if( match6)
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
                        fprintf( out->info_out, "%sPicked an IPV6 record (%s)\n", disp->line_pref, st);
		    }
		}
                else
                {
                    status->err_msg = strdup( EMSG_NO_IPS_IN_HOSTRECS);
                    *rc = ERR_GETHOST_FAILED;
                    if( out->debug_level >= DEBUG_MEDIUM2)
                    {
                        if( runex->client_ip_type == IP_V6) fprintf( out->info_out,
                          "%sIPV6 host record required, none found.\n", disp->line_pref);
                        else if( runex->client_ip_type == IP_V4) fprintf( out->info_out,
                          "%sIPV4 host record required, none found.\n", disp->line_pref);
                        else fprintf( out->info_out,
                          "%sNo IPV4 or IPV6 host records found.\n", disp->line_pref);
		    }
		}

                freeaddrinfo( hostrecs);
	    }
	}

        /* This case can only be true if this routine was called without an IPv4 address,
         *   an IPv6 address, or a hostname to lookup. In other words, this shouldn't
         *   happen if the previous code checked the request for the minimum info required.
         */
        if( *rc == RC_NORMAL && status->ip_type == IP_UNKNOWN)
        {
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "%sOops! no idea what to lookup.\n", disp->line_pref);
            status->err_msg = strdup( EMSG_NOTHING_TO_LOOKUP);
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
    int sock_type, sock, sysrc, salen = 0, dev_prot;
#ifndef linux
    int flags;
#endif
    char display_ip[ INET6_ADDRSTRLEN + 1], *st, *strc;
    struct fetch_status *status = 0;
    struct output_options *out = 0;
    struct sockaddr *sa = 0;
    struct linger linger;
    struct pollfd netbox;
    struct exec_controls *runex = 0;
    struct sockaddr_in sock4;
    struct sockaddr_in6 sock6;
    struct display_settings *disp = 0;

    memset( &sock4, EOS_CH, sizeof sock4);
    memset( &sock6, EOS_CH, sizeof sock6);

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        out = plan->out;
        runex = plan->run;
        disp = plan->disp;
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
            status->err_msg = sys_call_fail_msg( "socket");
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
                status->err_msg = sys_call_fail_msg( "inet_pton");
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
                status->err_msg = sys_call_fail_msg( "inet_pton");
                *rc = ERR_INVALID_DATA;
	    }
	}
    }

    if( *rc == RC_NORMAL && runex->device_summ && !sa)
    {
        dev_prot = runex->device_summ->addr->sa_family;
        if( dev_prot == AF_INET6)
        {
            sa = (struct sockaddr *) &sock6;
            memcpy( &sock6, (struct sockaddr_in6 *) runex->device_summ->addr, sizeof sock6);
            sock6.sin6_port = 0;
            sock6.sin6_flowinfo = 0;
            salen = sizeof sock6;
            if( out->debug_level >= DEBUG_MEDIUM3)
            {
                st = display_ip;
                memset( display_ip, EOS_CH, (sizeof display_ip));
                strc = (char *) inet_ntop( AF_INET6, &sock6.sin6_addr, st, (sizeof display_ip));
                if( !strc) strcat( display_ip, INVALID_IP);
                fprintf( out->info_out, "Bind to interface /dev/%s ipv6: %s\n",
                  runex->device_summ->name, st);
            }
        }
        else if( dev_prot == AF_INET)
        {
            sa = (struct sockaddr *) &sock4;
            memcpy( &sock4, (struct sockaddr_in *) runex->device_summ->addr, sizeof sock4);
            sock4.sin_port = 0;
            salen = sizeof sock4;
            if( out->debug_level >= DEBUG_MEDIUM3)
            {
                st = display_ip;
                memset( display_ip, EOS_CH, (sizeof display_ip));
                strc = (char *) inet_ntop( AF_INET, &sock4.sin_addr, st, (sizeof display_ip));
                if( !strc) strcat( display_ip, INVALID_IP);
                fprintf( out->info_out, "Bind to interface /dev/%s ipv4: %s\n",
                  runex->device_summ->name, st);
            }
        }
        else *rc = ERR_UNSUPPORTED; /* should never happen, but set an error message just in case */
    }

    if( *rc == RC_NORMAL && sa)
    {
        sysrc = bind( sock, sa, salen);
        if( sysrc == -1)
        {
            status->end_errno = errno;
            status->err_msg = sys_call_fail_msg( "bind");
            *rc = ERR_SYS_CALL;
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
            status->err_msg = strdup( EMSG_LINGER_WONT);
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
            status->err_msg = strdup( EMSG_NONBLOCK_WONT);
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

            if( out->debug_level >= DEBUG_HIGH3) debug_timelog( out->info_out, disp->line_pref, "Pre connect-to-server");
            sysrc = poll( &netbox, 1, runex->conn_timeout);

            if( out->debug_level >= DEBUG_HIGH3) debug_timelog( out->info_out, disp->line_pref, "Aft connect-to-server");
            if( sysrc == 0)
            {
                status->err_msg = strdup( EMSG_CONNECT_POLL_TIMEOUT);
                *rc = ERR_POLL_TIMEOUT;
	    }
            else if( sysrc == -1)
            {
                status->end_errno = errno;
                status->err_msg = sys_call_fail_msg( "poll");
                *rc = ERR_SYS_CALL;
	    }
            else
            {
               if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
                 "%sConnect worked, sock=%d, type=%d\n", disp->line_pref, sock, status->ip_type);
               status->last_state |= LS_ESTAB_CONNECT;
            }
	}
        else
        {
            status->end_errno = errno;
            status->err_msg = sys_call_fail_msg( "connect");
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
    struct output_options *out = 0;
    struct display_settings *disp = 0;

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        out = plan->out;
        disp = plan->disp;
    }

    if( *rc == RC_NORMAL)
    {
        sysrc = write( status->conn_sock, status->request, status->request_len);
        if( sysrc == status->request_len)
        {
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "%sWrote the entire request successfully.\n", disp->line_pref);
            status->last_state |= LS_SENT_REQUEST;
	}
        else
        {
            status->end_errno = errno;
            status->last_state |= LS_NO_REQUEST;
            status->err_msg = sys_call_fail_msg( "write");
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
    struct exec_controls *runex = 0;
    struct display_settings *disp = 0;

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        out = plan->out;
        runex = plan->run;
        disp = plan->disp;
    }

    if( *rc == RC_NORMAL)
    {
        netbox.fd = status->conn_sock;
        netbox.events = POLL_EVENTS_READ;
        netbox.revents = 0;

        if( out->debug_level >= DEBUG_HIGH3) debug_timelog( out->info_out, disp->line_pref, "Pre wait-for-reply");
        sysrc = poll( &netbox, 1, runex->conn_timeout);
        if( out->debug_level >= DEBUG_HIGH3) debug_timelog( out->info_out, disp->line_pref, "Aft wait-for-reply");
        if( sysrc == 0)
        {
            status->err_msg = strdup( EMSG_REPLY_POLL_TIMEOUT);
            *rc = ERR_POLL_TIMEOUT;
            status->last_state |= LS_NO_RESPONSE;
	}
        else if( sysrc == -1)
        {
            status->end_errno = errno;
            status->err_msg = sys_call_fail_msg( "poll");
            *rc = ERR_SYS_CALL;
            status->last_state |= LS_NO_RESPONSE;
	}
        else
        {
            if( out->debug_level >= DEBUG_MEDIUM2) fprintf( out->info_out,
              "%sSocket is ready for read() calls.\n", disp->line_pref);
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
    struct exec_controls *runex = 0;
    struct display_settings *disp = 0;

    buff = (char *) malloc( buffsize);
    if( !buff) *rc = ERR_MALLOC_FAILED;

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        out = plan->out;
        runex = plan->run;
        disp = plan->disp;
    }

    if( *rc == RC_NORMAL)
    {
        netbox.fd = status->conn_sock;
        netbox.events = POLL_EVENTS_READ;
        netbox.revents = 0;

        for( done = 0; !done && *rc == RC_NORMAL; )
        {
            if( out->debug_level >= DEBUG_HIGH3) debug_timelog( out->info_out, disp->line_pref, "Pre pull-response");
            sysrc = poll( &netbox, 1, runex->conn_timeout);
            if( out->debug_level >= DEBUG_HIGH3) debug_timelog( out->info_out, disp->line_pref, "Aft pull-response");
            if( sysrc == 0)
            {
                status->err_msg = strdup( EMSG_READ_POLL_TIMEOUT);
                *rc = ERR_POLL_TIMEOUT;
	    }
            else if( sysrc == -1)
            {
                status->end_errno = errno;
                status->err_msg = sys_call_fail_msg( "poll");
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
                          out->info_out, "%sIgnoring EINTR return from poll()\n", disp->line_pref);
		    }
                    else
                    {
                        status->end_errno = errno;
                        status->err_msg = sys_call_fail_msg( "read");
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
                        fprintf( out->info_out, "%sRead %d bytes (", disp->line_pref, packlen);
                        for( pos = buff; pos < buff + packlen; pos++)
                          if( *pos == GT_CH)
                          {
                              if( out->out_html) fprintf( out->info_out, HTML_GT_ESCAPE);
                              else fputc( *pos, out->info_out);
			  }
                          else if( *pos == LT_CH)
                          {
                              if( out->out_html) fprintf( out->info_out, HTML_LT_ESCAPE);
                              else fputc( *pos, out->info_out);
			  }
                          else if( isprint( *pos) || *pos == LF_CH) fputc( *pos, out->info_out);
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
    char *sng_pos = 0, sng_patt[ 3] = { LF_CH, LF_CH, EOS_CH };
    char *dbl_pos = 0, dbl_patt[ 5] = { CR_CH, LF_CH, CR_CH, LF_CH, EOS_CH };
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

void stats_from_packets( int *rc, struct plan_data *plan, int iter)

{
    int npackets = 0, packsize_sum = 0, off, is_last_data;
#ifdef SHOW_GORY_XFRATE_CALC_DETAILS
    int dbgfirst = 0, initial_size = 0;
    float r1, r2;
    char m1, m2;
#endif
    int *packsize = 0;
    float readlag_sum = 0.0, xfrate_sum = 0.0, packsize_mean = 0.0,
      readlag_mean = 0.0, xfrate_mean = 0.0, mean_diff = 0.0,
      elap = 0.0, rate = 0.0, packsize_max, readlag_max, xfrate_max,
      packsize_msd_norm, readlag_msd_norm, xfrate_msd_norm,
      packsize_mth_norm, readlag_mth_norm, xfrate_mth_norm,
      packsize_mfo_norm, readlag_mfo_norm, xfrate_mfo_norm,
      adj, adj2, samp_sz, samp_div, variance;
    float *readlag = 0, *xfrate = 0;
    struct summary_stats *profile = 0;
    struct display_settings *display = 0;
    struct fetch_status *status = 0;
    struct ckpt_chain *walk = 0, *start = 0, *prevpack = 0;
    struct payload_breakout *breakout = 0;

    if( *rc == RC_NORMAL)
    {
        display = plan->disp;
        profile = plan->profile;
        status = plan->status;
        breakout = plan->partlist;
    }

    if( *rc == RC_NORMAL) if( display->show_timers)
    {
        profile->lookup_time = 0.0;
        profile->connect_time = 0.0;
        profile->request_time = 0.0;
        profile->response_time = 0.0;
        profile->complete_time = 0.0;

        if( iter == 1)
        {
            profile->lookup_sum = 0.0;
            profile->connect_sum = 0.0;
            profile->request_sum = 0.0;
            profile->response_sum = 0.0;
            profile->close_sum = 0.0;
            profile->complete_sum = 0.0;
            profile->xfer_sum = 0;
            profile->payload_sum = 0;
        }

        walk = status->checkpoint;

        for( ; walk; walk = walk->next)
        {
            if( start) elap = calc_time_difference( &start->clock, &walk->clock, status->clock_res);
            else elap = 0.0;

            if( walk->event == EVENT_START_FETCH) start = walk;
            else if( walk->event == EVENT_DNS_LOOKUP) profile->lookup_time = elap;
            else if( walk->event == EVENT_CONNECT_SERVER) profile->connect_time = elap;
            else if( walk->event == EVENT_REQUEST_SENT) profile->request_time = elap;
            else if( walk->event == EVENT_FIRST_RESPONSE) profile->response_time = elap;
            else if( walk->event == EVENT_READ_ALL_DATA) profile->complete_time = elap;
            else if( walk->event == EVENT_READ_PACKET && walk->detail) npackets++;
	}

        if( npackets)
        {
            readlag = (float *) malloc( npackets * (sizeof *readlag));
            if( !readlag) *rc = ERR_MALLOC_FAILED;

            if( *rc == RC_NORMAL)
            {
                packsize = (int *) malloc( npackets * (sizeof *packsize));
                if( !packsize) *rc = ERR_MALLOC_FAILED;
	    }

            if( *rc == RC_NORMAL)
            {
                xfrate = (float *) malloc( npackets * (sizeof *packsize));
                if( !xfrate) *rc = ERR_MALLOC_FAILED;
	    }

            if( *rc == RC_NORMAL) for( off = 0; off < npackets; off++)
            {
                *(packsize + off) = 0;
                *(readlag + off) = 0.0;
                *(xfrate + off) = 0.0;
	    }
	}

        packsize_max = 0.0;
        readlag_max = 0.0;
        xfrate_max = 0.0;
        walk = status->checkpoint;
        is_last_data = 0;

        for( off = 0; walk; walk = walk->next) if( walk->event == EVENT_READ_PACKET && walk->detail)
        {
            if( !walk->next) is_last_data = 1;
            else if( walk->next->event != EVENT_READ_PACKET) is_last_data = 1;

            /* The last packet is usually partial, so it should be ignored */
            if( !is_last_data)
            {
#ifdef SHOW_GORY_XFRATE_CALC_DETAILS
                if( !off) initial_size = walk->detail->len;
#endif
                packsize_sum += walk->detail->len;
                *(packsize + off) = walk->detail->len;
                if( (float) walk->detail->len > packsize_max) packsize_max = (float) walk->detail->len;
	    }

            /* Lag is calculate from the last read, so the first packet has none */
            if( off)
            {
                elap = calc_time_difference( &prevpack->clock, &walk->clock, status->clock_res);
                readlag_sum += elap;
                *(readlag + off - 1) = elap;
                if( elap > readlag_max) readlag_max = elap;
	    }

            /* Per packet transfer rates only make sense for the 2nd to next to last packets */
            if( off && !is_last_data)
            {
                if( !walk->detail->len || elap == 0.0) rate = 0.0;
                else rate = ((float) walk->detail->len) / elap;
                xfrate_sum += rate;
                *(xfrate + off - 1) = rate;
                if( rate > xfrate_max) xfrate_max = rate;

#ifdef SHOW_GORY_XFRATE_CALC_DETAILS
                if( !dbgfirst++)
                {
                    printf( "dbg:: Xf_Sum Seq.  Elapsed  Size      Xfer   Xfer-Rate-Sum  SzSum   Read-Lag-Sum  Run-Xfer\n");
                    printf( "dbg:: Xf_Sum ---. -------- ----- --------- --------------- ------ -------------- ---------\n");
                }
                r1 = get_scaled_number( &m1, rate);
                r2 = get_scaled_number( &m2, (packsize_sum - initial_size) / readlag_sum);
                printf( "dbg:: Xf_Sum %3d. %8.6f %5d %8.3f%c %15.4f %6d %14.6f %8.3f%c\n", off, elap, walk->detail->len, 
                  r1, m1, xfrate_sum, packsize_sum - initial_size, readlag_sum, r2, m2);
#endif
	    }

            prevpack = walk;
            off++;
	}

        if( !npackets)
        {
           readlag_mean = packsize_mean = xfrate_mean = 0.0;
	}
        else if( npackets == 1)
        {
            readlag_mean = readlag_sum;
            packsize_mean = (float) packsize_sum;
            xfrate_mean = 0.0;
	}
        else
        {
            npackets--;
            readlag_mean = readlag_sum / npackets;
            packsize_mean = ((float) packsize_sum) / npackets;
            if( npackets == 1) xfrate_mean = xfrate_sum;
            else xfrate_mean = xfrate_sum / (npackets - 1);
	}

        packsize_msd_norm = readlag_msd_norm = xfrate_msd_norm = 0.0;
        packsize_mth_norm = readlag_mth_norm = xfrate_mth_norm = 0.0;
        packsize_mfo_norm = readlag_mfo_norm = xfrate_mfo_norm = 0.0;
        walk = status->checkpoint;
#ifdef SHOW_GORY_XFRATE_CALC_DETAILS
        printf( "dbg:: - - -\n");
#endif

        for( off = 0; off < npackets; off++)
        {
            if( packsize_max)
            {
                mean_diff = (((float) *(packsize + off)) - packsize_mean) / packsize_max;
                adj = mean_diff * mean_diff;
                packsize_msd_norm += adj;
                adj *= mean_diff;
                packsize_mth_norm += adj;
                adj *= mean_diff;
                packsize_mfo_norm += adj;
	    }

            if( readlag_max)
            {
                mean_diff = (*(readlag + off) - readlag_mean) / readlag_max;
                adj = mean_diff * mean_diff;
                readlag_msd_norm += adj;
                adj *= mean_diff;
                readlag_mth_norm += adj;
                adj *= mean_diff;
                readlag_mfo_norm += adj;
	    }

            if( off < npackets - 1 && xfrate_max)
            {
                mean_diff = (*(xfrate + off) - xfrate_mean) / xfrate_max;
                adj = mean_diff * mean_diff;
                xfrate_msd_norm += adj;
                adj *= mean_diff;
                xfrate_mth_norm += adj;
                adj *= mean_diff;
                xfrate_mfo_norm += adj;
	    }

/*
            mean_diff = ((float) *(packsize + off)) - packsize_mean;
            packsize_msd_sum += (mean_diff * mean_diff);

            mean_diff = *(readlag + off) - readlag_mean;
            readlag_msd_sum += (mean_diff * mean_diff);

            if( off < npackets - 1)
            {
                mean_diff = *(xfrate + off) - xfrate_mean;
                xfrate_msd_sum += (mean_diff * mean_diff);
	    }
#ifdef SHOW_GORY_XFRATE_CALC_DETAILS
            printf( "dbg:: Xf_RMS %3d. %f %f\n", off, mean_diff * mean_diff, xfrate_msd_sum);
#endif
 */
	}

#ifdef SHOW_DEBUG_STATS_CALCS
        printf( "dbg:: - - - %d\n", npackets);
        printf( "dbg:: What:        Sum       Mean        Max MSqDiffSum MThDiffSum MFoDiffSum\n");
        printf( "dbg:: ----- ---------- ---------- ---------- ---------- ---------- ----------\n");
        printf( "dbg:: Lag:: %10.3e %10.3e %10.3e %10.3e %10.3e %10.3e\n", readlag_sum,
          readlag_mean, readlag_max, readlag_msd_norm, readlag_mth_norm, readlag_mfo_norm);
        printf( "dbg:: Pack: %10d %10.3e %10.3e %10.3e %10.3e %10.3e\n", packsize_sum,
          packsize_mean, packsize_max, packsize_msd_norm, packsize_mth_norm, packsize_mfo_norm);
        printf( "dbg:: XRat: %10.3e %10.3e %10.3e %10.3e %10.3e %10.3e\n", xfrate_sum,
          xfrate_mean, xfrate_max, xfrate_msd_norm, xfrate_mth_norm, xfrate_mfo_norm);
#endif

        if( npackets < 2)
        {
            profile->readlag_norm_stdev = 0.0;
            profile->packsize_norm_stdev = 0.0;
            profile->xfrate_norm_stdev = 0.0;
            profile->readlag_norm_skew = 0.0;
            profile->packsize_norm_skew = 0.0;
            profile->xfrate_norm_skew = 0.0;
            profile->readlag_norm_kurt = 0.0;
            profile->packsize_norm_kurt = 0.0;
            profile->xfrate_norm_kurt = 0.0;
	}
        else
        {
            /* When calculating the variance/stdev from a sample popultion you divide by N-1 */
            adj = (float) npackets - 1;
            profile->packsize_norm_stdev = sqrt( packsize_msd_norm / adj);
            profile->readlag_norm_stdev = sqrt( readlag_msd_norm / adj);
            if( npackets == 2) profile->xfrate_norm_stdev = 0.0;
            else profile->xfrate_norm_stdev = sqrt( xfrate_msd_norm / (adj - 1.0));

            if( npackets < 3)
            {
                profile->packsize_norm_skew = 0.0;
                profile->readlag_norm_skew = 0.0;
                profile->xfrate_norm_skew = 0.0;
                profile->packsize_norm_kurt = 0.0;
                profile->readlag_norm_kurt = 0.0;
                profile->xfrate_norm_kurt = 0.0;
	    }
            else
            {
                adj = (float) (npackets) / (float) ((npackets - 1) * (npackets - 2));

                adj2 = pow( profile->packsize_norm_stdev, 3.0);
                if( !adj2) profile->packsize_norm_skew = 0.0;
                else profile->packsize_norm_skew = adj * (packsize_mth_norm / adj2);

                adj2 = pow( profile->readlag_norm_stdev, 3.0);
                if( !adj2) profile->readlag_norm_skew = 0.0;
                else profile->readlag_norm_skew = adj * (readlag_mth_norm / adj2);

/*              profile->packsize_norm_skew = adj * (packsize_mth_norm / pow( profile->packsize_norm_stdev, 3.0));
 *              profile->readlag_norm_skew = adj * (readlag_mth_norm / pow( profile->readlag_norm_stdev, 3.0));
 */
                if( npackets < 4) profile->xfrate_norm_skew = 0.0;
                else
                {
                    adj = (float) (npackets - 1) / (float) ((npackets - 2) * (npackets - 3));

                    adj2 = pow( profile->xfrate_norm_stdev, 3.0);
                    if( !adj2) profile->xfrate_norm_skew = 0.0;
                    else profile->xfrate_norm_skew = adj * (xfrate_mth_norm / adj2);

/*                  profile->xfrate_norm_skew = adj * (xfrate_mth_norm / pow( profile->xfrate_norm_stdev, 3.0));
 */		}

                if( npackets < 4)
                {
                    profile->packsize_norm_kurt = 0.0;
                    profile->readlag_norm_kurt = 0.0;
                    profile->xfrate_norm_kurt = 0.0;
		}
                else
                {
                    samp_sz = (float) npackets;
                    samp_div = samp_sz - 1.0;
                    adj = (samp_sz * (samp_sz + 1)) / ( (samp_div * (samp_div - 1) * (samp_div - 2) ) );
                    adj2 = 3 * ( (samp_div * samp_div) / ( (samp_div - 1) * (samp_div - 2) ) );

                    variance = packsize_msd_norm / samp_div;
                    if( !variance) profile->packsize_norm_kurt = 0.0;
                    else profile->packsize_norm_kurt = adj * ( packsize_mfo_norm / (variance * variance) ) - adj2;
#ifdef SHOW_DEBUG_STATS_CALCS
                    printf( "dbg:: Stat: What: AdjFactor1 MFoDiffSum   Variance AdjFactor2       Kurt\n");
                    printf( "dbg:: ----- ----- ---------- ---------- ---------- ---------- ----------\n");
                    printf( "dbg:: Kurt: Pack: %10.3e %10.3e %10.3e %10.3e %10.3e\n", adj, packsize_mfo_norm,
                      variance, adj2, profile->packsize_norm_kurt);
#endif

                    variance = readlag_msd_norm / samp_div;
                    if( !variance) profile->readlag_norm_kurt = 0.0;
                    else profile->readlag_norm_kurt = adj * ( readlag_mfo_norm / (variance * variance) ) - adj2;
#ifdef SHOW_DEBUG_STATS_CALCS
                    printf( "dbg:: Kurt: Lag:: %10.3e %10.3e %10.3e %10.3e %10.3e\n", adj, readlag_mfo_norm,
                      variance, adj2, profile->readlag_norm_kurt);
#endif

                    if( npackets < 5) profile->xfrate_norm_kurt = 0.0;
                    else
                    {
                        samp_sz = (float) (npackets - 1);
                        samp_div = samp_sz - 1.0;
                        adj = (samp_sz * (samp_sz + 1)) / ( (samp_div * (samp_div - 1) * (samp_div - 2) ) );
                        adj2 = 3 * ( (samp_div * samp_div) / ( (samp_div - 1) * (samp_div - 2) ) );

                        variance = xfrate_msd_norm / samp_div;
                        if( !variance) profile->xfrate_norm_kurt = 0.0;
                        else profile->xfrate_norm_kurt = adj * ( xfrate_mfo_norm / (variance * variance) ) - adj2;
#ifdef SHOW_DEBUG_STATS_CALCS
                        printf( "dbg:: Kurt: XRat: %10.3e %10.3e %10.3e %10.3e %10.3e\n", adj, xfrate_mfo_norm,
                          variance, adj2, profile->xfrate_norm_kurt);
#endif
                    }
		}

#ifdef SHOW_DEBUG_STATS_CALCS
                printf( "dbg:: Stat: What: AdjFactor1 MThDiffSum     StdDev   StDev**3       Skew\n");
                printf( "dbg:: ----- ----- ---------- ---------- ---------- ---------- ----------\n");
                printf( "dbg:: Skew: Lag:: %10.3e %10.3e %10.3e %10.3e %10.3e\n",
                  (float) (npackets) / (float) ((npackets - 1) * (npackets - 2)), readlag_mth_norm,
                  profile->readlag_norm_stdev, pow( profile->readlag_norm_stdev, 3.0), profile->readlag_norm_skew);
                printf( "dbg:: Skew: Pack: %10.3e %10.3e %10.3e %10.3e %10.3e\n",
                  (float) (npackets) / (float) ((npackets - 1) * (npackets - 2)), packsize_mth_norm,
                  profile->packsize_norm_stdev, pow( profile->packsize_norm_stdev, 3.0), profile->packsize_norm_skew);
                printf( "dbg:: Skew: XRat: %10.3e %10.3e %10.3e %10.3e %10.3e\n", adj, xfrate_mth_norm,
                  profile->xfrate_norm_stdev, pow( profile->xfrate_norm_stdev, 3.0), profile->xfrate_norm_skew);
#endif
	    }
	}

        profile->packsize_mean = packsize_mean;
        profile->readlag_mean = readlag_mean;
        profile->xfrate_mean = xfrate_mean;
        profile->lookup_sum += profile->lookup_time;
        profile->connect_sum += profile->connect_time - profile->lookup_time;
        profile->request_sum += profile->request_time - profile->connect_time;
        profile->response_sum += profile->response_time - profile->request_time;
        profile->close_sum += profile->complete_time - profile->response_time;
        profile->complete_sum += profile->complete_time;
        profile->xfer_sum += status->response_len;
        profile->payload_sum += status->response_len - breakout->header_size;
        profile->packsize_stdev_sum += profile->packsize_norm_stdev;
        profile->packsize_skew_sum += profile->packsize_norm_skew;
        profile->packsize_kurt_sum += profile->packsize_norm_kurt;
        profile->readlag_stdev_sum += profile->readlag_norm_stdev;
        profile->readlag_skew_sum += profile->readlag_norm_skew;
        profile->readlag_kurt_sum += profile->readlag_norm_kurt;
        profile->xfrate_stdev_sum += profile->xfrate_norm_stdev;
        profile->xfrate_skew_sum += profile->xfrate_norm_skew;
        profile->xfrate_kurt_sum += profile->xfrate_norm_kurt;
    }

    if( packsize) free( packsize);
    if( readlag) free( readlag);
    if( xfrate) free( xfrate);

    return;
}

/* --- */

char *string_from_data_blocks( struct ckpt_chain *st_block, char *st_pos, struct ckpt_chain *en_block,
  char *en_pos)

{
    int size = 0, partial;
    char *line = 0, *st = 0;
    struct ckpt_chain *walk = 0;

    if( st_block == en_block)
    {
        size = (en_pos - st_pos);
        if( size <= 0) size = 0;
        else size++;
    }
    else
    {
        size = st_block->detail->len - (st_pos - st_block->detail->data);
        if( size < 0) size = 0;
        
        if( size) for( walk = st_block->next; walk && walk != en_block; walk = walk->next)
          if( walk->detail) size += walk->detail->len;

        if( size)
        {
            partial = en_pos - en_block->detail->data;
            if( partial < 0) size = 0;
            else size += partial + 1;
        }
    }

    if( size)
    {
        line = (char *) malloc( size + 1);
    }

    if( line)
    {
        if( st_block == en_block)
        {
            memcpy( line, st_pos, size);
            *(line + size) = EOS_CH;
	}
        else
        {
            partial = st_block->detail->len - (st_pos - st_block->detail->data);
            st = line;
            memcpy( st, st_pos, partial);
            st+= partial;

            for( walk = st_block->next; walk && walk != en_block; walk = walk->next)
            {
                if( walk->detail)
                {
                    memcpy( st, walk->detail->data, walk->detail->len);
                    st += walk->detail->len;
                }
            }

            partial = en_pos - en_block->detail->data + 1;
            memcpy( st, en_block->detail->data, partial);
            st += partial;
            *st = EOS_CH;
	}
    }

    return( line);
}

/* --- */

int split_out_header_lines( struct ckpt_chain *chain, struct payload_breakout *breakout,
  char *prefix)

{
    int result = RC_NORMAL, nhead, datalen, off;
/*
int dbg_size = 0;
char *dbg_st = 0;
struct data_block *dbg_xx = 0;
 */
    char *st = 0, *eoh = 0, *st_pos = 0, *line = 0, *pos = 0;
    struct ckpt_chain *walk = 0, *st_block = 0;
    struct data_block *headset = 0;

    if( !chain || !breakout) result = ERR_UNSUPPORTED;
    else
    {
        eoh = breakout->head_spot->position;

        walk = chain;
        nhead = 0;

        for( st = 0; st != eoh; )
        {
            if( walk->detail)
            {
                st = walk->detail->data;
                datalen = walk->detail->len;
                for( off = 0; off <= datalen && walk; off++)
                {
                    if( *st == LF_CH) nhead++;
                    if( st == eoh) walk = 0;
                    else st++;
		}
	    }

            if( st != eoh && walk) walk = walk->next;
	}

        if( !nhead) result = ERR_UNSUPPORTED;
        else
        {
/*
dbg_size = (sizeof *headset);
printf( "%sdbg:: headers:%d data-block-size:%d total:%d\n", prefix, nhead, dbg_size, dbg_size * nhead);
 */
            headset = (struct data_block *) malloc( (sizeof *headset) * nhead);
/* for( off = 0, dbg_xx = headset; off < nhead; off++, dbg_xx++) printf( "%sdbg:: pre--set %d. address:%p data:%p len:%d \n", prefix, off, dbg_xx, dbg_xx->data, dbg_xx->len); */
            if( !headset) result = ERR_MALLOC_FAILED;
            else for( off = 0; off < nhead; off++)
            {
                (headset + off)->len = 0;
                (headset + off)->data = 0;
	    }
/* for( off = 0, dbg_xx = headset; off < nhead; off++, dbg_xx++) printf( "%sdbg:: post-set %d. address:%p data:%p len:%d \n", prefix, off, dbg_xx, dbg_xx->data, dbg_xx->len); */
	}

        walk = chain;
        st_block = 0;
        st_pos = 0;
        nhead = 0;

        for( st = 0; result == RC_NORMAL && st != eoh; )
        {
            if( walk->detail)
            {
                st = walk->detail->data;
                datalen = walk->detail->len;
                for( off = 0; off <= datalen && walk; off++)
                {
                    if( !st_block)
                    {
                        st_block = walk;
                        st_pos = st;
		    }

                    if( *st == LF_CH)
                    {
                        line = string_from_data_blocks( st_block, st_pos, walk, st);
                        if( !line)
                        {
                            walk = 0;
                            result = ERR_MALLOC_FAILED;
			}
                        else
                        {
                            pos = index( line, CR_CH);
                            if( pos) *pos = EOS_CH;
                            pos = index( line, LF_CH);
                            if( pos) *pos = EOS_CH;
                            (headset + nhead)->data = line;
                            (headset + nhead)->len = strlen( line);
                            nhead++;

                            st_block = 0;
                            st_pos = 0;
			}
                    }

                    if( st == eoh) walk = 0;
                    else st++;
                }
            }

            if( st != eoh && walk) walk = walk->next;
        }

        breakout->n_headers = nhead;
        breakout->header_line = headset;
    }

    return( result);
}

/* --- */

struct http_status_response *parse_http_status( char *line)

{
    int code, err = 0;
    char *st = 0, *delim = 0, *pos = 0;
    struct http_status_response *resp = 0;

    resp = (struct http_status_response *) malloc( sizeof *resp);
    if( resp)
    {
        resp->code = 0;
        resp->version = 0;
        resp->reason = 0;

        delim = index( line, BLANK_CH);
        st = dup_memory( line, delim - 1);
        if( st)
        {
            resp->version = st;

            pos = delim + 1;
            delim = index( pos, BLANK_CH);
            st = dup_memory( pos, delim - 1);
            if( st)
            {
                code = strtol( st, &pos, 10);
                if( !pos) err = 1;
                else if( *pos) err = 1;
                else
                {
                    resp->code = code;
                    pos = strdup( delim + 1);
                    if( pos) resp->reason = pos;
		}

                free( st);
	    }
        }

        if( !resp->version || !resp->reason || err)
        {
            if( resp->version) free( resp->version);
            if( resp->reason) free( resp->reason);
            free( resp);
            resp = 0;
	}
    }

    return( resp);
}

/* --- */

int parse_http_response( struct payload_breakout *breakout)

{
    int result = RC_NORMAL;
    char *response = 0;

    if( !breakout) result = ERR_UNSUPPORTED;
    else
    {
        response = breakout->header_line->data;
        if( !response) result = ERR_UNSUPPORTED;
        else if( !*response) result = ERR_UNSUPPORTED;
        else
        {
            breakout->response_status = parse_http_status( response);
            if( !breakout->response_status) result = ERR_UNSUPPORTED;
	}
    }

    if( result == RC_NORMAL)
    {
        breakout->trans_encoding = find_http_header( breakout, HTTP_HEAD_TRANSFER_ENCODING);
        breakout->content_type = find_http_header( breakout, HTTP_HEAD_CONTENT_TYPE);
    }

    return( result);
}

/* --- */

char *find_http_header( struct payload_breakout *breakout, char *which)

{
    int hsize, off;
    char *result = 0, *st = 0;
    struct data_block *headset;

    if( which) if( *which)
    {
        hsize = strlen( which);

        headset = breakout->header_line;
        for( off = 0; !result && off < breakout->n_headers; off++)
        {
            if( !strncasecmp( which, (headset + off)->data, hsize))
            {
                st = (headset + off)->data + hsize;
                if( *st == COLON_CH)
                {
                    for( st++; *st == BLANK_CH; st++) ;
                    if( *st) result = st;
		}
	    }
	}
    }

    return( result);
}

/* --- */

int find_header_size( struct payload_breakout *breakout, struct ckpt_chain *chain)

{
    int size = 0;
    char *fence = 0;
    struct ckpt_chain *walk = 0, *last = 0;

    if( breakout && chain) if( breakout->head_spot)
    {
        last = breakout->head_spot->chain;
        fence = breakout->head_spot->position;

        for( walk = chain; walk && walk != last; walk = walk->next)
          if( walk->detail) size += walk->detail->len;

        if( walk == last) size += fence - walk->detail->data;
    }

    return( size);
}

/* --- */

void parse_payload( int *rc, struct plan_data *plan)

{
    int off, nh;
    struct fetch_status *status = 0;
    struct payload_breakout *breakout = 0;
    struct data_block *header;
    struct display_settings *disp = 0;

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        breakout = plan->partlist;
        disp = plan->disp;

        breakout->head_spot = find_header_break( status->checkpoint);
        if( !breakout->head_spot)
        {
            status->err_msg = strdup( EMSG_HTTP_HEADER_NO_END);
            *rc = ERR_UNSUPPORTED;
	}
        else
        {
            breakout->header_size = find_header_size( breakout, status->checkpoint);
	}
    }

    if( *rc == RC_NORMAL) *rc = split_out_header_lines( status->checkpoint, breakout, disp->line_pref);

    if( *rc == RC_NORMAL)
    {
        nh = breakout->n_headers;
        header = breakout->header_line;

        if( plan->out->debug_level >= DEBUG_MEDIUM2) for( off = 0; off < nh; off++)
          fprintf( plan->out->info_out, "%s%d. Header '%s'\n", disp->line_pref, off, (header + off)->data);
    }

    if( *rc == RC_NORMAL)
    {
        *rc = parse_http_response( breakout);

        if( plan->out->debug_level >= DEBUG_MEDIUM2) fprintf( plan->out->info_out,
          "- - - Parsed HTTP headers:\nContent-Type: '%s'\nTransfer-Encoding: '%s'\n",
          SPSP( breakout->content_type), SPSP( breakout->trans_encoding));
    }

    return;
}

/* --- */

void display_output( int *rc, struct plan_data *plan, int iter)

{
    int packlen = 0, in_head, disp_time_len = TIME_DISPLAY_SIZE, sysrc,
      dns_ms, conn_ms, send_ms, resp_ms, close_ms, complete_ms,
      xfer_mean = 0, http_rc, is_chunked = 0, chunk_left, build_len,
      inc_len, payload_mean = 0;
    long top, now_sec = 0, now_sub = 0, prev_sec = 0, prev_sub = 0,
      diff_sec = 0, diff_sub = 0;
    float elap = 0.0, nloop = 0.0, totrate, datarate, pre_response;
    char *pos = 0, *last_pos = 0, datarate_mark, totrate_mark;
    char disp_time[ TIME_DISPLAY_SIZE], xdig_conv[ 2] = { '\0', '\0'};
    struct fetch_status *status = 0;
    struct output_options *out = 0;
    struct exec_controls *runex = 0;
    struct ckpt_chain *walk = 0, *before = 0;
    struct summary_stats *profile = 0;
    struct data_block *detail = 0;
    struct display_settings *display = 0;
    struct payload_breakout *breakout = 0;
    struct chain_position *head_spot = 0;
    struct tm local_wall_start;

    if( *rc == RC_NORMAL)
    {
        status = plan->status;
        out = plan->out;
        display = plan->disp;
        runex = plan->run;
        breakout = plan->partlist;
        profile = plan->profile;
        top = (long) (1.0 / status->clock_res);
        head_spot = breakout->head_spot;
    }

    if( *rc == RC_NORMAL && display->show_timers)
    {
        if( iter == 1 && display->show_timerheaders)
        {
            if( display->show_number)
            {
                fprintf( out->info_out, "%s%s\n", TIME_SUMMARY_HEADER_SEQ_1, TIME_SUMMARY_HEADER_1);
                fprintf( out->info_out, "%s%s\n", TIME_SUMMARY_HEADER_SEQ_2, TIME_SUMMARY_HEADER_2);
                fprintf( out->info_out, "%s%s\n", TIME_SUMMARY_HEADER_SEQ_3, TIME_SUMMARY_HEADER_3);
	    }
            else
            {
                fprintf( out->info_out, "%s\n", TIME_SUMMARY_HEADER_1);
                fprintf( out->info_out, "%s\n", TIME_SUMMARY_HEADER_2);
                fprintf( out->info_out, "%s\n", TIME_SUMMARY_HEADER_3);
	    }
	}

        if( profile->complete_time) totrate = get_scaled_number( &totrate_mark,
          (float) status->response_len / (float) profile->complete_time);
        else
        {
            totrate = 0.0;
            totrate_mark = 'b';
	}

        (void) localtime_r( &status->wall_start, &local_wall_start);
        disp_time[ 0] = EOS_CH;
        sysrc = strftime( disp_time, disp_time_len, TIME_DISPLAY_FORMAT, &local_wall_start);
        if( !sysrc)
        {
            status->end_errno = errno;
            status->err_msg = sys_call_fail_msg( "strftime");
            *rc = ERR_SYS_CALL;
	}
    }

    if( *rc == RC_NORMAL && display->show_timers)
    {
        dns_ms = lroundf( profile->lookup_time * 1000.0);
        conn_ms = lroundf( (profile->connect_time - profile->lookup_time) * 1000.0);
        send_ms = lroundf( (profile->request_time - profile->connect_time) * 1000.0);
        resp_ms = lroundf( (profile->response_time - profile->request_time) * 1000.0);
        close_ms = lroundf( (profile->complete_time - profile->response_time) * 1000.0);
        complete_ms = lroundf( profile->complete_time * 1000.0);

        if( profile->complete_time - profile->response_time)
        {
            datarate = get_scaled_number( &datarate_mark,
              status->response_len / (profile->complete_time - profile->response_time));
	}
        else
        {
            datarate_mark = 'b';
            datarate = 0.0;
	}

        if( !plan->partlist->response_status) http_rc = 999;
        else http_rc = plan->partlist->response_status->code;

        if( display->show_number) fprintf( out->info_out, "%3d. ", iter);
        fprintf( out->info_out, "%s %3d %5d %5d %5d %5d %5d %5d %6ld %6ld %5.1f%c %5.1f%c %7.5f %9.2e %9.2e %7.5f %9.2e %9.2e %7.5f %9.2e %9.2e\n",
          disp_time, http_rc, dns_ms, conn_ms, send_ms, resp_ms, close_ms, complete_ms,
          status->response_len, status->response_len - breakout->header_size,
          totrate, totrate_mark, datarate, datarate_mark,
          profile->packsize_norm_stdev, profile->packsize_norm_skew, profile->packsize_norm_kurt,
          profile->readlag_norm_stdev, profile->readlag_norm_skew, profile->readlag_norm_kurt,
          profile->xfrate_norm_stdev, profile->xfrate_norm_skew, profile->xfrate_norm_kurt);
    }

    if( *rc == RC_NORMAL && display->show_packetime)
    {
        if( display->show_number) fprintf( out->info_out, "%3d. ", iter);
        fprintf( out->info_out, "Packets:");

        before = 0;
        walk = status->checkpoint;

        for( ; walk; walk = walk->next)
        {
            if( walk->event == EVENT_READ_PACKET && walk->detail)
            {
                if( !before) before = walk;
                elap = calc_time_difference( &before->clock, &walk->clock, status->clock_res);

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

                packlen = walk->detail->len;
                fprintf( out->info_out, " %ld/%d", lroundf( elap * 1000000.0), packlen);

                before = walk;
	    }
	}
        fprintf( out->info_out, "\n");
    }

    if( *rc == RC_NORMAL && display->show_timers)
    {
        if( iter == runex->loop_count)
        {
            nloop = runex->loop_count / 1000.0;

            if( !nloop) 
            {
                dns_ms = 0;
                conn_ms = 0;
                send_ms = 0;
                resp_ms = 0;
                close_ms = 0;
                complete_ms = 0;
                xfer_mean = 0;
                payload_mean = 0;
	    }
            else
            {
                dns_ms = lroundf( profile->lookup_sum / nloop);
                conn_ms = lroundf( profile->connect_sum / nloop);
                send_ms = lroundf( profile->request_sum / nloop);
                resp_ms = lroundf( profile->response_sum / nloop);
                close_ms = lroundf( profile->close_sum / nloop);
                complete_ms = lroundf( profile->complete_sum / nloop);
                xfer_mean = profile->xfer_sum / runex->loop_count;
                payload_mean = profile->payload_sum / runex->loop_count;
	    }

            if( !profile->complete_sum)
            {
                totrate = 0.0;
                totrate_mark = 'b';
	    }
            else totrate = get_scaled_number( &totrate_mark,
              (float) profile->xfer_sum / profile->complete_sum);

            if( !(profile->complete_sum - profile->response_sum))
            {
                datarate_mark = 'b';
                datarate = 0.0;
            }
            else
            {
                pre_response = profile->lookup_sum + profile->connect_sum + profile->request_sum + profile->response_sum;
                datarate = get_scaled_number( &datarate_mark,
                  profile->xfer_sum / (profile->complete_sum - pre_response));
	    }

            nloop = (float) runex->loop_count;
            fprintf( out->info_out, "Average values: ");
            if( display->show_number) fprintf( out->info_out, "  ---");
            fprintf( out->info_out, " %5d %5d %5d %5d %5d %5d %6d %6d %5.1f%c %5.1f%c %7.5f %9.2e %9.2e %7.5f %9.2e %9.2e %7.5f %9.2e %9.2e\n",
              dns_ms, conn_ms, send_ms, resp_ms, close_ms, complete_ms,
              xfer_mean, payload_mean, totrate, totrate_mark, datarate, datarate_mark,
              profile->packsize_stdev_sum / nloop, profile->packsize_skew_sum / nloop, profile->packsize_kurt_sum / nloop, 
              profile->readlag_stdev_sum / nloop, profile->readlag_skew_sum / nloop, profile->readlag_kurt_sum / nloop, 
              profile->xfrate_stdev_sum / nloop, profile->xfrate_skew_sum / nloop, profile->xfrate_kurt_sum / nloop);
        }
    }

    if( *rc == RC_NORMAL && (display->show_head || display->show_data))
    {
        in_head = 1;
        walk = status->checkpoint;

        if( display->show_data && breakout->trans_encoding)
          is_chunked = !strcasecmp( breakout->trans_encoding, ENC_TYPE_CHUNKED);

        if( is_chunked)
        {
            chunk_left = 0;
            build_len = 0;
	}

        if( out->out_html)
        {
            if( display->show_head)
            {
                printf( HTML_RESP_IFRAME_START, HTML_HEIGHT_HEAD);
                printf( "%s", HTML_PREFORMAT_START);
	    }
            else printf( HTML_RESP_IFRAME_START, HTML_HEIGHT_DATA);
	}

        for( ; walk && *rc == RC_NORMAL; walk = walk->next)
        {
            detail = walk->detail;
            if( detail) if( detail->len)
            {
                pos = detail->data;
                last_pos = pos + detail->len;
                for( ; pos < last_pos; pos++)
                {
                    if( in_head && display->show_head) fputc( *pos, stdout);
                    else if( !in_head && display->show_data)
                    {
                        if( is_chunked && !chunk_left)
                        {
                            if( *pos == LF_CH)
                            {
                                chunk_left = build_len;
                                build_len = 0;
			    }
                            else if( isxdigit( *pos))
                            {
                                xdig_conv[ 0] = *pos;
                                errno = 0;
                                inc_len = strtoul( xdig_conv, 0, HEX_BASE);
                                if( errno) *rc = ERR_SYS_CALL;
                                else
                                {
/*
int dbg_build;
dbg_build = (build_len * 16) + inc_len;
printf( "::dbg Build up chunk len, pos'%c' build: %d = %d + %d\n", *pos, dbg_build,
  build_len * 16, inc_len);
 */
                                    build_len = (build_len * 16) + inc_len;
				}
			    }
                            else if( *pos != CR_CH) *rc = ERR_BAD_FORMAT;
			}
                        else
                        {
                            if( !out->out_html) fputc( *pos, stdout);
                            else if( *pos == DQUOTE_CH) printf( "%s", HTML_DQ_ESCAPE);
                            else if( *pos == AMPER_CH) printf( "%s", HTML_AM_ESCAPE);
                            else fputc( *pos, stdout);
                            if( is_chunked) chunk_left--;
			}
		    }

                    if( pos == head_spot->position)
                    {
                        in_head = 0;
                        if( out->out_html && display->show_head && display->show_data)
                        {
                            printf( "%s%s", HTML_PREFORMAT_END, HTML_RESP_IFRAME_END);
                            printf( HTML_RESP_IFRAME_START, HTML_HEIGHT_DATA);
			}
		    }
		}
	    }
 	}

        if( out->out_html) printf( "%s", HTML_RESP_IFRAME_END);
    }

    return;
}

/* --- */

int construct_request( struct plan_data *plan)

{
    int rc = RC_NORMAL, empty, ex_len;
    char *blank = EMPTY_STRING, *webhost = 0, *prefhost = 0, *agent = DEFAULT_FETCH_USER_AGENT,
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
            *ex_headers = EOS_CH;

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
        walk->from = PATT_HTTP_PROTOCOL;
        if( target->http_protocol == USE_HTTP11) walk->to = PROT_HTTP11;
        else walk->to = PROT_HTTP10;
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

    if( rc == RC_NORMAL) if( out->debug_level >= DEBUG_HIGH2)
    {
        fprintf( out->info_out,
          "- - - HTTP request - - -\n%s\n- - -\n", fetch->request);
    }

    if( rc == RC_NORMAL) fetch->last_state |= LS_GEN_REQUEST;

    /* --- */

    return( rc);
}

/* --- */

struct plan_data *allocate_hf_plan_data()

{
    int error = 0;
    struct plan_data *plan = 0;
    struct target_info *target = 0;
    struct display_settings *disp = 0;
    struct exec_controls *runex = 0;
    struct output_options *out = 0;
    struct fetch_status *status = 0;
    struct ckpt_chain *checkpoint = 0;
    struct payload_breakout *breakout = 0;
    struct summary_stats *profile = 0;
    struct http_status_response *resp_status = 0;

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

    if( !error)
    {
        breakout = (struct payload_breakout *) malloc( sizeof *breakout);
        if( !breakout) error = 1;
    }

    if( !error)
    {
        profile = (struct summary_stats *) malloc( sizeof *profile);
        if( !profile) error = 1;
    }

    if( !error)
    {
        resp_status = (struct http_status_response *) malloc( sizeof *resp_status);
        if( !resp_status) error = 1;
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
        if( breakout) free( breakout);
        if( profile) free( profile);
        if( resp_status) free( resp_status);
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
        plan->partlist = breakout;
        plan->profile = profile;

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
        target->pref_protocol = 0;
        target->http_protocol = USE_HTTP11;

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
        disp->line_pref = strdup( EMPTY_STRING);

        runex->loop_count = 0;
        runex->loop_pause = 0;
        runex->conn_timeout = 0;
        runex->client_ip_type = IP_UNKNOWN;
        runex->client_ip = 0;
        runex->bind_interface = 0;
        runex->device_summ = 0;

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

        resp_status->code = 0;
        resp_status->version = 0;
        resp_status->reason = 0;

        breakout->head_spot = 0;
        breakout->n_headers = 0;
        breakout->header_size = 0;
        breakout->header_line = 0;
        breakout->content_type = 0;
        breakout->trans_encoding = 0;
        breakout->response_status = resp_status;

        profile->xfer_sum = 0;
        profile->payload_sum = 0;
        profile->lookup_time = 0.0;
        profile->lookup_sum = 0.0;
        profile->connect_time = 0.0;
        profile->connect_sum = 0.0;
        profile->request_time = 0.0;
        profile->request_sum = 0.0;
        profile->response_time = 0.0;
        profile->response_sum = 0.0;
        profile->close_sum = 0.0;
        profile->complete_time = 0.0;
        profile->complete_sum = 0.0;
        profile->packsize_mean = 0.0;
        profile->xfrate_mean = 0.0;
        profile->packsize_norm_stdev = 0.0;
        profile->readlag_norm_stdev = 0.0;
        profile->xfrate_norm_stdev = 0.0;
        profile->packsize_norm_skew = 0.0;
        profile->readlag_norm_skew = 0.0;
        profile->xfrate_norm_skew = 0.0;
        profile->packsize_norm_kurt = 0.0;
        profile->readlag_norm_kurt = 0.0;
        profile->xfrate_norm_kurt = 0.0;
        profile->packsize_stdev_sum = 0.0;
        profile->packsize_skew_sum = 0.0;
        profile->packsize_kurt_sum = 0.0;
        profile->readlag_stdev_sum = 0.0;
        profile->readlag_skew_sum = 0.0;
        profile->readlag_kurt_sum = 0.0;
        profile->xfrate_stdev_sum = 0.0;
        profile->xfrate_skew_sum = 0.0;
        profile->xfrate_kurt_sum = 0.0;
    }

    return( plan);
}

/* --- */

struct plan_data *figure_out_plan( int *returncode, int narg, char **opts)
{
    int rc = RC_NORMAL, errlen, in_val, nop_head = 0, nop_data = 0, nop_comp = 0, show_form = 0,
      is_cgi = 0, nop_prefprot = 0, nop_httpprot = 0, ip_pref, has_pref_clip = 0;
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
      { OP_TCP4,         OP_TYPE_FLAG, OP_FL_BLANK,   FL_TCP4,           0, DEF_TCP4,         0, 0 },
      { OP_TCP4,         OP_TYPE_FLAG, OP_FL_BLANK,   FL_TCP4_2,         0, DEF_TCP4,         0, 0 },
      { OP_TCP6,         OP_TYPE_FLAG, OP_FL_BLANK,   FL_TCP6,           0, DEF_TCP6,         0, 0 },
      { OP_TCP6,         OP_TYPE_FLAG, OP_FL_BLANK,   FL_TCP6_2,         0, DEF_TCP6,         0, 0 },
      { OP_HTTP10,       OP_TYPE_FLAG, OP_FL_BLANK,   FL_HTTP10,         0, DEF_HTTP10,       0, 0 },
      { OP_HTTP11,       OP_TYPE_FLAG, OP_FL_BLANK,   FL_HTTP11,         0, DEF_HTTP11,       0, 0 },
      { OP_INTERFACE,    OP_TYPE_CHAR, OP_FL_BLANK,   FL_INTERFACE,      0, DEF_INTERFACE,    0, 0 },
      { OP_INTERFACE,    OP_TYPE_CHAR, OP_FL_BLANK,   FL_INTERFACE_2,    0, DEF_INTERFACE,    0, 0 },
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
    struct interface_info *dev_info = 0;

    /* --- */

    plan = allocate_hf_plan_data();
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
            is_cgi = 1;
            out->info_out = stdout;
            out->err_out = stdout;
            out->out_html = 1;
            cgi_data = get_cgi_data( &rc);
            if( !cgi_data) show_form = 1;
            else if( !*cgi_data) show_form = 1;
            if( show_form)
            {
                fprintf( out->info_out, "%s\n", HTML_RESP_HEADER);
                display_entry_form();
                status->last_state |= LS_HTML_FORM_SENT;
                show_form = 1;
                rc = ERR_NOTHING_LEFT;
	    }
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

        if(( co = cond_get_matching_option( &rc, OP_HTML, opset, nflags)))
        {
            if( is_cgi && !(co->flags & OP_FL_FOUND)) out->out_html = 1;
            else out->out_html = *((int *) co->parsed);

            if( out->out_html) display->line_pref = strdup( HTML_BREAK_NOOP);
	}

        if(( co = cond_get_matching_option( &rc, OP_USESTDERR, opset, nflags)))
        {
            in_val = *((int *) co->parsed);
            if( in_val && !is_cgi) out->info_out = stderr;
        }

        if( rc != ERR_NOTHING_LEFT && out->out_html) fprintf( out->info_out, "%s\n%s\n", HTML_RESP_HEADER, HTML_PREFORMAT_START);

        if( rc == RC_NORMAL && out->debug_level >= DEBUG_HIGH1)
        {
            co = cond_get_matching_option( &rc, OP_DEBUG, opset, nflags);
            SHOW_OPT_IF_DEBUG( display->line_pref, "debug")

            co = cond_get_matching_option( &rc, OP_HTML, opset, nflags);
            SHOW_OPT_IF_DEBUG( display->line_pref, "html")

            co = cond_get_matching_option( &rc, OP_USESTDERR, opset, nflags);
            SHOW_OPT_IF_DEBUG( display->line_pref, "use-stderr")
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
            status->err_msg = strdup( EMSG_UNRECOG_OPTIONS);
            rc = ERR_SYNTAX;

            unrecognized = (char *) malloc( errlen + 1);
            if( !unrecognized) rc = ERR_MALLOC_FAILED;
            else
            {
                *unrecognized = EOS_CH;
                for( walk = extra_opts; walk; )
                {
                    if( walk->opt) strcat( unrecognized, walk->opt);
                    walk = walk->next;
                    if( walk) if( walk->opt) strcat( unrecognized, SPACE_STRING);
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
            fprintf( out->info_out, "%src=%d extra(", display->line_pref, rc);
            sep = EMPTY_STRING;
            for( walk = extra_opts; walk; )
            {
                fprintf( out->info_out, "%s%s", sep, walk->opt);
                walk = walk->next;
                if( walk) sep = SPACE_STRING;
                else sep = EMPTY_STRING;
            }
            fprintf( out->info_out, ")\n");
            print_option_settings( out->info_out, nflags, opset);
        }
    }

    /* ---
     */

    if(( co = cond_get_matching_option( &rc, OP_HEADER, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "header")
        display->show_head = *((int *) co->parsed);
        nop_head = co->opt_num;
    }

    if(( co = cond_get_matching_option( &rc, OP_OUTPUT, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "all-output")
        display->show_complete = *((int *) co->parsed);
        nop_comp = co->opt_num;
    }

    if(( co = cond_get_matching_option( &rc, OP_URI, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "URI")
        target->conn_uri = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_CONNHOST, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "connhost")
        target->conn_host = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_WEBSERVER, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "webhost")
        target->http_host = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_URL, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "URL")
        target->conn_url = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_PORT, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "port")
        target->conn_port = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_TIMERS, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "timers")
        display->show_timers = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_DATA, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "data")
        display->show_data = *((int *) co->parsed);
        nop_data = co->opt_num;
    }

    if(( co = cond_get_matching_option( &rc, OP_LOOP, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "loop")
        runex->loop_count = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_NUMBER, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "show-number")
        display->show_number = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_PAUSE, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "pause")
        runex->loop_pause = *((int *) co->parsed) * 1000;
    }

    if(( co = cond_get_matching_option( &rc, OP_TIMERHEADERS, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "timerheaders")
        display->show_timerheaders = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_TIMEOUT, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "timeout")
        runex->conn_timeout = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_PROXY, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "proxy")
        target->proxy_url = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_PACKETIME, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "show-packet")
        display->show_packetime = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_CLIENTIP, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "client-ip")
        runex->client_ip = (char *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_PTHRU, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "passthru")
        target->conn_pthru = *((int *) co->parsed);
    }

    if(( co = cond_get_matching_option( &rc, OP_XHEADER, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "extra-header")
        target->extra_headers = (struct value_chain *) co->parsed;
    }

    if(( co = cond_get_matching_option( &rc, OP_TCP4, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "tcp4")
        if( co->opt_num > nop_prefprot)
        {
            if( *((int *) co->parsed)) target->pref_protocol = IP_V4;
            else target->pref_protocol = IP_V6;
            nop_prefprot = co->opt_num;
	}
    }

    if(( co = cond_get_matching_option( &rc, OP_TCP6, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "tcp6")
        if( co->opt_num > nop_prefprot)
        {
            if( *((int *) co->parsed)) target->pref_protocol = IP_V6;
            else target->pref_protocol = IP_V4;
            nop_prefprot = co->opt_num;
	}
    }

    if(( co = cond_get_matching_option( &rc, OP_HTTP10, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "http1.0")
        if( co->opt_num > nop_httpprot)
        {
            if( *((int *) co->parsed)) target->http_protocol = USE_HTTP10;
            else target->http_protocol = USE_HTTP11;
            nop_httpprot = co->opt_num;
	}
    }

    if(( co = cond_get_matching_option( &rc, OP_HTTP11, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "http1.1")
        if( co->opt_num > nop_httpprot)
        {
            if( *((int *) co->parsed)) target->http_protocol = USE_HTTP11;
            else target->http_protocol = USE_HTTP10;
            nop_httpprot = co->opt_num;
	}
    }

    if(( co = cond_get_matching_option( &rc, OP_AUTH, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "auth")
        target->auth_user = strdup( (char *) co->parsed);
        st = index( target->auth_user, COLON_CH);
        if( st)
        {
            *st = EOS_CH;
            target->auth_passwd = strdup( st + 1);
        }
        else
        {
            target->auth_passwd = (char *) malloc( 1);
            if( target->auth_passwd) *target->auth_passwd = EOS_CH;
        }
        if( !target->auth_passwd) rc = ERR_MALLOC_FAILED;
    }

    if(( co = cond_get_matching_option( &rc, OP_INTERFACE, opset, nflags)))
    {
        SHOW_OPT_IF_DEBUG( display->line_pref, "interface")
        runex->bind_interface = (char *) co->parsed;
    }

    /* ---
     * If the options controlling which parts of the response conflict, then
     *   check where each was given on the command line to decide which
     *   overrides.
     */

    if( rc == RC_NORMAL)
    {
        if( !display->show_complete && (display->show_head || display->show_data))
        {
            if( nop_comp > nop_head) display->show_head = 0;
            if( nop_comp > nop_data) display->show_data = 0;
	}

        else if( display->show_complete && (!display->show_head || !display->show_data))
        {
            if( nop_comp > nop_head) display->show_head = 1;
            if( nop_comp > nop_data) display->show_data = 1;
	}
    }

    if( rc == RC_NORMAL)
    {
        if( runex->client_ip) if( *runex->client_ip)
        {
            has_pref_clip = 1;

            if( is_ipv6_address( runex->client_ip)) runex->client_ip_type = IP_V6;
            else if( is_ipv4_address( runex->client_ip)) runex->client_ip_type = IP_V4;

            if( runex->client_ip_type != IP_UNKNOWN && target->pref_protocol != IP_UNKNOWN
              && runex->client_ip_type != target->pref_protocol) rc = ERR_UNSUPPORTED;  /* Conflicting options... Set an error message */
	}
    }

    /* ---
     * If the user asked to bind to a specific interface, lookup the
     *   correct IP for that device based on the name and the IP protocol
     *   (IPv4 or IPv6) if a preference was stated explicitly.  Use IPv4
     *   by default since it's more likely to work.
     */

    if( rc == RC_NORMAL && !has_pref_clip && runex->bind_interface) if( *runex->bind_interface)
    {
        if( target->pref_protocol == IP_V4) ip_pref = AF_INET;
        else if( target->pref_protocol == IP_V6) ip_pref = AF_INET6;
        else ip_pref = AF_INET;

        dev_info = get_matching_interface( runex->bind_interface, ip_pref, IFF_UP);
        if( !dev_info) rc = ERR_UNSUPPORTED; /* No device matching name/protocol... Set an error message */
        else runex->device_summ = dev_info;
    }

    /* --- */

    if( rc == RC_NORMAL && runex->loop_count < 1) runex->loop_count = 1;

    *returncode = rc;

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
        status->err_msg = strdup( EMSG_UNRECOV_NULL_ANCHOR);
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
                status->err_msg = sys_call_fail_msg( "clock_gettime");
#else
                status->err_msg = sys_call_fail_msg( "gettimeofday");
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

    if( plan)
    {
        target = plan->target;
        disp = plan->disp;
        runex = plan->run;
        out = plan->out;
        fetch = plan->status;
    }

    if( rc == RC_NORMAL) if( disp->show_help)
    {
        fprintf( out->info_out, "%s", MSG_SHOW_SYNTAX);
        rc = ERR_NOTHING_LEFT;
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
        fprintf( out->info_out, "- - - - pref-protocol: (%d)\n", target->pref_protocol);

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
        fprintf( out->info_out, "- - - - client-ip-type: %d\n", runex->client_ip_type);
        fprintf( out->info_out, "- - - - client-ip: (%s)\n", SPSP( runex->client_ip));
        fprintf( out->info_out, "- - - - interface: (%s)\n", SPSP( runex->bind_interface));

        fprintf( out->info_out, "\n- - FetchStatus:\n");
        fprintf( out->info_out, "- - - - last-state: %d\n", fetch->last_state);
        fprintf( out->info_out, "- - - - errmsg: (%s)\n", SPSP( fetch->err_msg));
        fprintf( out->info_out, "- - - - request: (%s)\n", SPSP( fetch->request));
    }

    if( rc == RC_NORMAL) rc = execute_fetch_plan( plan);

    /* --- */

    if( rc == ERR_NOTHING_LEFT) rc = RC_NORMAL;

    if( rc != RC_NORMAL)
    {
        emsg = fetch->err_msg;
        if( !emsg) emsg = UNDEFINED_ERROR;
        else if( !*emsg) emsg = UNDEFINED_ERROR;

        fprintf( out->err_out, "%sError(%d/%06x): %s. %s\n", disp->line_pref, rc, fetch->last_state, cli_strerror( rc), emsg);
    }

    if( out) if( out->debug_level >= DEBUG_MEDIUM1) fprintf( out->info_out,
      "%sEnd state: %06X, End err: %d\n", disp->line_pref, fetch->last_state, fetch->end_errno);

    if( out->out_html) fprintf( out->info_out, "%s\n%s\n", HTML_PREFORMAT_END, HTML_RESP_END);

    /* --- */

    exit( rc);
}
