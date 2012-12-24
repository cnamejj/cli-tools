#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include "parse_opt.h"
#include "send-udp-message.h"
#include "err_ref.h"

/* --- */

struct task_details *allocate_plan_data()

{
    struct task_details *plan = 0;

    plan = (struct task_details *) malloc( (sizeof *plan));

    if( plan)
    {
        plan->use_ip = 0;
        plan->dest_port = 0;
        plan->found_family = 0;
        plan->debug = 0;
        plan->dest_host = 0;
        plan->message = 0;
        plan->err_msg = 0;
        plan->dest4.sin_family = AF_INET;
        plan->dest4.sin_port = 0;
        plan->dest4.sin_addr.s_addr = 0;
        plan->dest6.sin6_family = AF_INET6;
        plan->dest6.sin6_port = 0;
        plan->dest6.sin6_flowinfo = 0;
        plan->dest6.sin6_addr = in6addr_loopback;
        plan->dest6.sin6_scope_id = SCOPE_LINK;
    }

    return( plan);
}

/* --- */

struct option_set *get_matching_option( int flag_num, struct option_set *opset, int nflags)

{
    struct option_set *walk, *match = 0;
    int off;

    for( off= 0, walk = opset; off < nflags; off++, walk++)
    {
        if( walk->num == flag_num)
        {
            if( !match) match = walk;
            else if( walk->opt_num > match->opt_num) match = walk;
	}
    }

    return( match);
}

/* --- */

int parse_destination_value( struct task_details *plan, char *destination)

{
    int rc = RC_NORMAL, hostlen, port, errlen;
    char *st = 0, *last_colon, *invalid = 0, *host = 0;

    /* Save the address of the last colon found, which we need to split the
     * hostname (or IP address) from the port.
     */
    for( st = destination, last_colon = 0; *st; st++)
      if( *st == ':') last_colon = st;

    if( last_colon > destination)
    {
        hostlen = last_colon - destination;
        host = (char *) malloc( hostlen + 1);
        if( !host) rc = ERR_MALLOC_FAILED;
        else
        {
            memcpy( host, destination, hostlen);
            *(host + hostlen) = '\0';
            st = last_colon + 1;
            if( !*st) rc = ERR_INVALID_DATA;
            else
            {
                port = strtol( st, &invalid, 10);
                if( *invalid) rc = ERR_INVALID_DATA;
	    }
	}
    }
    else rc = ERR_INVALID_DATA;

    if( rc == RC_NORMAL)
    {
        plan->dest_host = host;
        plan->dest_port = port;
    }
    else if( rc == ERR_INVALID_DATA)
    {
        errlen = strlen( ERRMSG_BAD_DEST) + strlen( destination);
        plan->err_msg = (char *) malloc( errlen);
        if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
        else snprintf( plan->err_msg, errlen, ERRMSG_BAD_DEST, destination);
    }
    
    return( rc);
}

/* --- */

struct task_details *figure_out_what_to_do( int *returncode, int narg, char **opts)

{
    int rc = RC_NORMAL, off, *int_p = 0, dest_set = 0;
    struct task_details *plan = 0;
    struct option_set opset[] = {
      { OP_DEST_BOTH, OP_TYPE_CHAR, OP_FL_BLANK, FL_DEST_BOTH, 0, DEF_DEST_BOTH, 0, 0 },
      { OP_DEST_BOTH, OP_TYPE_CHAR, OP_FL_BLANK, FL_DEST_BOTH_2, 0, DEF_DEST_BOTH, 0, 0 },
      { OP_DEST_BOTH, OP_TYPE_CHAR, OP_FL_BLANK, FL_DEST_BOTH_3, 0, DEF_DEST_BOTH, 0, 0 },
      { OP_MESSAGE, OP_TYPE_CHAR, OP_FL_BLANK, FL_MESSAGE, 0, DEF_MESSAGE, 0, 0 },
      { OP_MESSAGE, OP_TYPE_CHAR, OP_FL_BLANK, FL_MESSAGE_2, 0, DEF_MESSAGE, 0, 0 },
      { OP_MESSAGE, OP_TYPE_CHAR, OP_FL_BLANK, FL_MESSAGE_3, 0, DEF_MESSAGE, 0, 0 },
      { OP_PORT, OP_TYPE_INT, OP_FL_BLANK, FL_PORT, 0, DEF_PORT, 0, 0 },
      { OP_PORT, OP_TYPE_INT, OP_FL_BLANK, FL_PORT_2, 0, DEF_PORT, 0, 0 },
      { OP_HOST, OP_TYPE_CHAR, OP_FL_BLANK, FL_HOST, 0, DEF_HOST, 0, 0 },
      { OP_HOST, OP_TYPE_CHAR, OP_FL_BLANK, FL_HOST_2, 0, DEF_HOST, 0, 0 },
      { OP_IPV4, OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV4, 0, DEF_IPV4, 0, 0 },
      { OP_IPV4, OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV4_2, 0, DEF_IPV4, 0, 0 },
      { OP_IPV6, OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV6, 0, DEF_IPV6, 0, 0 },
      { OP_IPV6, OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV6_2, 0, DEF_IPV6, 0, 0 },
      { OP_DEBUG, OP_TYPE_INT, OP_FL_BLANK, FL_DEBUG, 0, DEF_DEBUG, 0, 0 },
    };
    struct option_set *co = 0, *ipv4 = 0, *ipv6 = 0;
    struct word_chain *extra_opts = 0, *walk = 0;
    int nflags = (sizeof opset) / (sizeof opset[0]);
    
    char *sep = 0;

    plan = allocate_plan_data();
    if( !plan) rc = ERR_MALLOC_FAILED;

    /* --- */
   
    if( rc == RC_NORMAL)
    {
        extra_opts = parse_command_options( &rc, opset, nflags, narg, opts);

        /* --- */

        if( plan->debug >= DEBUG_HIGH)
        {
            fprintf( stderr, "rc=%d extra(", rc);
            sep = "";
            for( walk = extra_opts; walk; )
            {
                fprintf( stderr, "%s%s", sep, walk->opt);
                walk = walk->next;
                if( walk) sep = " ";
                else sep = "";
            }
            fprintf( stderr, ")\n");

            fprintf( stderr, "Seq Num Typ Fl Opt\n");

            for( off= 0; off < nflags; off++)
            {
                co = opset + off;
                if( co->opt_num)
                {
                    fprintf( stderr, "%2d. %3d %3d %2x ", off + 1, co->num, co->type, co->flags);
                    fprintf( stderr, "%3d ", co->opt_num);
                    if( co->type == OP_TYPE_INT || co->type == OP_TYPE_FLAG)
                    {
                        int_p = (int *) co->parsed;
                        fprintf( stderr, "%d ", (int) *int_p);
                    }
                    else if( co->type == OP_TYPE_CHAR) fprintf( stderr, "(%s) ", (char *) co->parsed);
                    else fprintf( stderr, "/?/ ");
                    fprintf( stderr, "(%s) (%s) ", co->name, co->val);
                    fprintf( stderr, "\n");
                }
            }
            fprintf( stderr, "Seq Num Typ Fl Opt\n");

            for( off= 0; off < nflags; off++)
            {
                co = opset + off;
                fprintf( stderr, "%2d. %3d %3d %2x ", off + 1, co->num, co->type, co->flags);
                fprintf( stderr, "%3d ", co->opt_num);
                if( co->type == OP_TYPE_INT || co->type == OP_TYPE_FLAG)
                {
                    int_p = (int *) co->parsed;
                    fprintf( stderr, "%d ", (int) *int_p);
                }
                else if( co->type == OP_TYPE_CHAR) fprintf( stderr, "(%s) ", (char *) co->parsed);
                else fprintf( stderr, "/?/ ");
                fprintf( stderr, "(%s) (%s) ", co->name, co->val);
                fprintf( stderr, "\n");
            }
        }
    }

    /* --- */

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_DEST_BOTH, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            dest_set = co->opt_num;
            rc = parse_destination_value( plan, (char *) co->parsed);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_MESSAGE, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else plan->message = (char *) co->parsed;
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_PORT, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->opt_num >= dest_set) plan->dest_port = *((int *) co->parsed);
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_HOST, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->opt_num >= dest_set) plan->dest_host = (char *) co->parsed;
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_IPV4, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->flags == OP_FL_SET) plan->use_ip |= DO_IPV4;
        else plan->use_ip &= ~DO_IPV4;
        ipv4 = co;
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_IPV6, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->flags == OP_FL_SET) plan->use_ip |= DO_IPV6;
        else plan->use_ip &= ~DO_IPV6;
        ipv6 = co;
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_DEBUG, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else plan->debug = *((int *) co->parsed);
    }

/* ...figure out how to deal with "--no-" options for host, port and destination flags... */
/* ...also while it's a bit of a hack, if "-ipv6" is specified w/o "-ipv4", then assume
   we only want IPV6 addresses, meaning don't require "+ipv4 -ipv6".  Alternatively, adding
   a "-prefer [v6 | v4]" option might be cleaner... */

    /* By default both IPV4 and IPV6 are disabled.  If neither option was selected via
     * command line flags, them turn the both on so the code will take whichever one it
     * finds.
     */

    if( !(plan->use_ip & (DO_IPV4 | DO_IPV6)))
    {
        if( ipv4->opt_num && ipv6->opt_num) rc = ERR_INVALID_DATA;
        else if( ipv4->opt_num) plan->use_ip = DO_IPV6;
        else if( ipv6->opt_num) plan->use_ip = DO_IPV4;
        else plan->use_ip = DO_IPV4 | DO_IPV6;
    }

    if( rc == ERR_OPT_CONFIG) plan->err_msg = "Unrecoverable internal configuration error, can't continue.";
    else if( rc == ERR_INVALID_DATA) plan->err_msg = "Both --no-ipv4 and --no-ipv6 specified, at least one needs to be allowed.";

    *returncode = rc;

    return( plan);
}

/* --- */

int get_destination_ip( struct task_details *plan)

{
    int rc = RC_NORMAL, sysrc, errlen;
    struct addrinfo hints, *host_recs = 0, *walk, *match4 = 0, *match6 = 0;
    struct sockaddr_in *sa4 = 0;
    struct sockaddr_in6 *sa6 = 0;

    /* --- */

    plan->dest6.sin6_family = AF_INET6;
    plan->dest6.sin6_port = 0;
    plan->dest6.sin6_flowinfo = 0;
    plan->dest6.sin6_addr = in6addr_loopback;
    plan->dest6.sin6_scope_id = SCOPE_LINK;

    plan->dest4.sin_family = AF_INET;
    plan->dest4.sin_port = 0;
    plan->dest4.sin_addr.s_addr = 0;

    /* --- */

    sysrc = inet_pton( AF_INET, plan->dest_host, &plan->dest4.sin_addr);
    if( sysrc == 1)
    {
        plan->found_family = AF_INET;
        if( plan->debug >= DEBUG_MEDIUM) fprintf( stderr, "Found '%s' is a V4 IP address.\n", plan->dest_host);
    }
    else if( sysrc) rc = ERR_OPT_CONFIG;
    else
    {
        sysrc = inet_pton( AF_INET6, plan->dest_host, &plan->dest6.sin6_addr);
        if( sysrc == 1)
        {
            plan->found_family = AF_INET6;
            if( plan->debug >= DEBUG_MEDIUM) fprintf( stderr, "Found '%s' is a V6 IP address.\n", plan->dest_host);
	}
        else if( sysrc) rc = ERR_OPT_CONFIG;
        else
        {
            if( plan->debug >= DEBUG_MEDIUM) fprintf( stderr, "Looks like a hostname, need to look it up.\n");
            hints.ai_flags = AI_CANONNAME;
            if( (plan->use_ip & DO_IPV4) && (plan->use_ip & DO_IPV6)) hints.ai_family = AF_UNSPEC;
            else if( plan->use_ip & DO_IPV6) hints.ai_family = AF_INET6;
            else hints.ai_family = AF_INET;
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            hints.ai_addrlen = 0;
            hints.ai_addr = 0;
            hints.ai_canonname = 0;
            hints.ai_next = 0;

            sysrc = getaddrinfo( plan->dest_host, 0, &hints, &host_recs);
            if( !sysrc)
            {
                if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "...getaddrinfo() call worked, scan the host records to find the one we want...\n");
                match4 = match6 = 0;
                for( walk = host_recs; walk; )
                {
                    if( plan->debug >= DEBUG_NOISY) fprintf( stderr, "hr --> %x %x/%x/%x %x %x %d %s\n",
                      walk->ai_flags, walk->ai_family, AF_INET, AF_INET6, walk->ai_socktype, 
                      walk->ai_protocol, walk->ai_addrlen, walk->ai_canonname);

                    if( !match4 && walk->ai_family == AF_INET) match4 = walk;
                    if( !match6 && walk->ai_family == AF_INET6) match6 = walk;
                    if( match4 && match6) walk = 0;
                    else walk = walk->ai_next;
		}

                if( plan->use_ip & DO_IPV4 && match4)
                {
                    sa4 = (struct sockaddr_in *) match4->ai_addr;
                    plan->dest4.sin_addr = sa4->sin_addr;
                    plan->found_family = AF_INET;
		}
                else if( plan->use_ip & DO_IPV6 && match6)
                {
                    sa6 = (struct sockaddr_in6 *) match6->ai_addr;
                    plan->dest6.sin6_addr = sa6->sin6_addr;
                    plan->found_family = AF_INET6;
                    plan->dest6.sin6_scope_id = sa6->sin6_scope_id;
		}
                if( plan->debug >= DEBUG_NOISY) fprintf( stderr, "match4:%d/%d match6:%d/%d\n", !!match4, !!sa4, !!match6, !!sa6);
	    }
            else
            {
                if( sysrc == EAI_MEMORY) rc = ERR_MALLOC_FAILED;
                else rc = ERR_GETHOST_FAILED;
	    }
	}
    }

    if( !strcmp( plan->dest_host, IPV6_LOOPBACK_ADDRESS)) plan->dest6.sin6_scope_id = SCOPE_LOOP;
    else if( IN6_IS_ADDR_LINKLOCAL( &plan->dest6.sin6_addr)) plan->dest6.sin6_scope_id = SCOPE_LINK;
    else if( IN6_IS_ADDR_SITELOCAL( &plan->dest6.sin6_addr)) plan->dest6.sin6_scope_id = SCOPE_SITE;
    else plan->dest6.sin6_scope_id = SCOPE_GLOBAL;

    if( rc == ERR_GETHOST_FAILED)
    {
        errlen = strlen( ERRMSG_GETHOST_FAILED) + strlen( plan->dest_host);
        plan->err_msg = (char *) malloc( errlen);
        if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
        else snprintf( plan->err_msg, errlen, ERRMSG_GETHOST_FAILED, plan->dest_host);
    }
    else if( rc == ERR_OPT_CONFIG)
    {
        errlen = strlen( ERRMSG_INET_PTON) + strlen( plan->dest_host);
        plan->err_msg = (char *) malloc( errlen);
        if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
        else snprintf( plan->err_msg, errlen, ERRMSG_INET_PTON, plan->dest_host);
    }

    return( rc);
}

/* --- */

int main( int narg, char **opts)

{
    int rc = RC_NORMAL, opt_on = 1, sysrc, sock, destlen, msglen, errlen;
    struct sockaddr *dest = 0;
    char *chrc = 0, *err_msg = 0;
    char display_ip[ IP_DISPLAY_SIZE];
    struct task_details *plan = 0;
    void *s_addr;

    /* --- */

    plan = figure_out_what_to_do( &rc, narg, opts);

    if( rc == RC_NORMAL)
    {
        if( plan->debug >= DEBUG_LOW) fprintf( stderr, "\nPlan: host(%s) port(%d) ipv4(%d) ipv6(%d) msg(%s)\n",
          plan->dest_host, plan->dest_port, plan->use_ip & DO_IPV4, plan->use_ip & DO_IPV6, plan->message);
    }

    /* --- */

    if( rc == RC_NORMAL)
    {
        rc = get_destination_ip( plan);

        if( rc == RC_NORMAL && !plan->found_family)
        {
            rc = ERR_GETHOST_FAILED;
            errlen = strlen( ERRMSG_GETHOST_FAILED) + strlen( plan->dest_host);
            plan->err_msg = (char *) malloc( errlen);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
            else snprintf( plan->err_msg, errlen, ERRMSG_GETHOST_FAILED, plan->dest_host);
	}
    }

    if( rc == RC_NORMAL)
    {
        if( plan->found_family == AF_INET)
        {
            plan->dest4.sin_port = htons( plan->dest_port);
            dest = (struct sockaddr *) &plan->dest4;
            destlen = (sizeof plan->dest4);
            s_addr = &plan->dest4.sin_addr;
	}
        else
        {
            plan->dest6.sin6_port = htons( plan->dest_port);
            dest = (struct sockaddr *) &plan->dest6;
            destlen = (sizeof plan->dest6);
            s_addr = &plan->dest6.sin6_addr;
	}
        chrc = (char *) inet_ntop( plan->found_family, s_addr, display_ip, IP_DISPLAY_SIZE);

        if( !chrc)
        {
            rc = ERR_SYS_CALL;
            errlen = strlen( ERRMSG_INET_NTOP) + INT_ERR_DISPLAY_LEN;
            plan->err_msg = (char *) malloc( errlen);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
            else snprintf( plan->err_msg, errlen, ERRMSG_INET_NTOP, errno);
	}
        else if( plan->debug > DEBUG_LOW) fprintf( stderr, "Dest(%s) IP(%s)\n", plan->dest_host, display_ip);
    }

    if( rc == RC_NORMAL)
    {
        if( plan->debug >= DEBUG_NOISY) fprintf( stderr, "ff:%d type:%d\n", plan->found_family, SOCK_DGRAM);

        sock = socket( plan->found_family, SOCK_DGRAM, 0);
        if( sock == -1)
        {
            rc = ERR_SYS_CALL;
            errlen = strlen( ERRMSG_SOCKET_CALL) + INT_ERR_DISPLAY_LEN;
            plan->err_msg = (char *) malloc( errlen);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
            else snprintf( plan->err_msg, errlen, ERRMSG_SOCKET_CALL, errno);
	}
        else if( plan->found_family == AF_INET6)
        {
            sysrc = setsockopt( sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt_on, (sizeof opt_on));
            if( sysrc)
            {
                rc = ERR_SYS_CALL;
                errlen = strlen( ERRMSG_SETSOCKOPT_CALL) + INT_ERR_DISPLAY_LEN;
                plan->err_msg = (char *) malloc( errlen);
                if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
                else snprintf( plan->err_msg, errlen, ERRMSG_SETSOCKOPT_CALL, errno);
            }
	}
    }

    if( rc == RC_NORMAL)
    {
        msglen = strlen( plan->message);
        if( plan->debug >= DEBUG_NOISY) fprintf( stderr, "sock:%d mlen:%d dlen:%d dtype:%d dport:%d msg(%s)\n",
          sock, msglen, destlen, ((struct sockaddr_in *)dest)->sin_family, 
          ntohs(((struct sockaddr_in *)dest)->sin_port), plan->message);

        sysrc = sendto( sock, plan->message, msglen, 0, dest, destlen);
        if( sysrc == -1)
        {
            rc = ERR_SYS_CALL;
            errlen = strlen( ERRMSG_SENDTO_FAIL) + INT_ERR_DISPLAY_LEN;
            plan->err_msg = (char *) malloc( errlen);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
            else snprintf( plan->err_msg, errlen, ERRMSG_SENDTO_FAIL, errno);
	}
        else if( sysrc != msglen)
        {
            rc = ERR_SYS_CALL;
            errlen = strlen( ERRMSG_SENDTO_PARTIAL) + INT_ERR_DISPLAY_LEN * 2;
            plan->err_msg = (char *) malloc( errlen);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
            else snprintf( plan->err_msg, errlen, ERRMSG_SENDTO_PARTIAL, sysrc, msglen);
	}
        else
        {
            printf( "A %d character UDP message sent to %s (%s) port %d.\n",
              msglen, plan->dest_host, display_ip, plan->dest_port);
	}
    }

    /* --- */

    if( rc != RC_NORMAL)
    {
        fprintf( stderr, "Error, rc=%d\n", rc);
        if( !plan) err_msg = cli_strerror( rc);
        else if( !plan->err_msg) err_msg = cli_strerror( rc);
        else err_msg = plan->err_msg;
        fprintf( stderr, "%s\n", err_msg);
        if( errno) fprintf( stderr, "System error message: %s\n", strerror( errno));
    }

    exit( rc);
}
