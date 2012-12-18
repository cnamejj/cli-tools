#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include "parse_opt.h"
#include "send-udp-message.h"

/* --- */

#define DEBUG_OPTION_PARSING

#define ERRMSG_BAD_DEST "Destination '%s' invalid, expected 'host:port'."
#define ERRMSG_MALLOC_FAILED "Memory allocation failed."
#define ERRMSG_OPT_CONFIG "Unrecoverable configuration error."
#define ERRMSG_GETHOST_FAILED "Can't lookup hostname."
#define ERRMSG_SYS_CALL "A system call failed."

#define IP_DISPLAY_SIZE INET6_ADDRSTRLEN

#define IPV6_LOOPBACK_ADDRESS "::1"

/* -- Can't use this constants from "in6.h", so hardcode tem
#define SCOPE_LOOP __IPV6_ADDR_SCOPE_INTFACELOCAL
#define SCOPE_LINK __IPV6_ADDR_SCOPE_LINKLOCAL
#define SCOPE_SITE __IPV6_ADDR_SCOPE_SITELOCAL
#define SCOPE_GLOBAL __IPV6_ADDR_SCOPE_GLOBAL
 */
#define SCOPE_LOOP 0x01
#define SCOPE_LINK 0x02
#define SCOPE_SITE 0x05
#define SCOPE_GLOBAL 0x0e


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
    else if( rc == ERR_MALLOC_FAILED) plan->err_msg = ERRMSG_MALLOC_FAILED;
    
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
    };
    struct option_set *co = 0;
    struct word_chain *extra_opts = 0, *walk = 0;
    int nflags = (sizeof opset) / (sizeof opset[0]);
    
    char *sep = 0;

    plan = allocate_plan_data();
    if( !plan) rc = ERR_MALLOC_FAILED;


    /* --- */
   
    if( rc == RC_NORMAL)
    {
        extra_opts = parse_command_options( &rc, opset, nflags, narg, opts);

#ifdef DEBUG_OPTION_PARSING
        /* --- */

        printf( "rc=%d extra(", rc);
        sep = "";
        for( walk = extra_opts; walk; )
        {
            printf( "%s%s", sep, walk->opt);
            walk = walk->next;
            if( walk) sep = " ";
            else sep = "";
        }
        printf( ")\n");

        printf( "Seq Num Typ Fl Opt\n");

        for( off= 0; off < nflags; off++)
        {
            co = opset + off;
            if( co->opt_num)
            {
                printf( "%2d. %3d %3d %2x ", off + 1, co->num, co->type, co->flags);
                printf( "%3d ", co->opt_num);
                if( co->type == OP_TYPE_INT || co->type == OP_TYPE_FLAG)
                {
                    int_p = (int *) co->parsed;
                    printf( "%d ", (int) *int_p);
                }
                else if( co->type == OP_TYPE_CHAR) printf( "(%s) ", (char *) co->parsed);
                else printf( "/?/ ");
                printf( "(%s) (%s) ", co->name, co->val);
                printf( "\n");
            }
        }

        printf( "Seq Num Typ Fl Opt\n");

        for( off= 0; off < nflags; off++)
        {
            co = opset + off;
            printf( "%2d. %3d %3d %2x ", off + 1, co->num, co->type, co->flags);
            printf( "%3d ", co->opt_num);
            if( co->type == OP_TYPE_INT || co->type == OP_TYPE_FLAG)
            {
                int_p = (int *) co->parsed;
                printf( "%d ", (int) *int_p);
            }
            else if( co->type == OP_TYPE_CHAR) printf( "(%s) ", (char *) co->parsed);
            else printf( "/?/ ");
            printf( "(%s) (%s) ", co->name, co->val);
            printf( "\n");
        }
    }
#endif

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
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_IPV6, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->flags == OP_FL_SET) plan->use_ip |= DO_IPV6;
        else plan->use_ip &= ~DO_IPV6;
    }

/* ...figure out how to deal with "--no-" options for host, port and destination flags... */

    if( rc == ERR_OPT_CONFIG && plan) plan->err_msg = ERRMSG_OPT_CONFIG;

    *returncode = rc;

    return( plan);
}

/* --- */

int get_destination_ip( struct task_details *plan)

{
    int rc = RC_NORMAL, sysrc;
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
printf( "Found '%s' is a V4 IP address.\n", plan->dest_host);
    }
    else if( sysrc) { printf( "...wtf?...\n"); exit( 1); }
    else
    {
        sysrc = inet_pton( AF_INET6, plan->dest_host, &plan->dest6.sin6_addr);
        if( sysrc == 1)
        {
            plan->found_family = AF_INET6;
printf( "Found '%s' is a V6 IP address.\n", plan->dest_host);
	}
        else if( sysrc) { printf( "...wtf?...\n"); exit( 1); }
        else
        {
printf( "Looks like a hostname, need to look it up.\n");
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
printf( "...getaddrinfo() call worked, scan the host records to find the one we want...\n");
                match4 = match6 = 0;
                for( walk = host_recs; walk; )
                {
/*
printf( "hr --> %x %x/%x/%x %x %x %d %s\n",
  walk->ai_flags, walk->ai_family, AF_INET, AF_INET6, walk->ai_socktype, walk->ai_protocol, walk->ai_addrlen, walk->ai_canonname);
 */
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
printf( "match4:%d/%d match6:%d/%d\n", !!match4, !!sa4, !!match6, !!sa6);
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

    if( rc == ERR_MALLOC_FAILED) plan->err_msg = ERRMSG_MALLOC_FAILED;
    else if( rc == ERR_GETHOST_FAILED) plan->err_msg = ERRMSG_GETHOST_FAILED;

    return( rc);
}

/* --- */

int main( int narg, char **opts)

{
    int rc = RC_NORMAL, opt_on = 1, sysrc, sock, destlen, msglen;
    struct sockaddr *dest = 0;
    char *chrc = 0;
    char display_ip[ IP_DISPLAY_SIZE];
    struct task_details *plan = 0;
    void *s_addr;

    /* --- */

    plan = figure_out_what_to_do( &rc, narg, opts);

    if( rc == RC_NORMAL)
    {
        printf( "\nPlan: host(%s) port(%d) ipv4(%d) ipv6(%d) msg(%s)\n", plan->dest_host, plan->dest_port, plan->use_ip & DO_IPV4,
          plan->use_ip & DO_IPV6, plan->message);
    }
    else
    { 
        printf( "Error, rc=%d\n", rc);
        if( plan) if( plan->err_msg) printf( "%s\n", plan->err_msg);
    }

    /* --- */

    rc = get_destination_ip( plan);

    if( !plan->found_family)
    {
        if( rc == RC_NORMAL)
        {
           rc = ERR_GETHOST_FAILED;
           plan->err_msg = ERRMSG_GETHOST_FAILED;
	}
    }
    else if( rc == RC_NORMAL)
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
        
/*
        if( plan->found_family == AF_INET) chrc = (char *) inet_ntop( AF_INET, &plan->dest4.sin_addr, display_ip, IP_DISPLAY_SIZE);
        else chrc = (char *) inet_ntop( AF_INET6, &plan->dest6.sin6_addr, display_ip, IP_DISPLAY_SIZE);
 */

        if( !chrc)
        {
            rc = ERR_SYS_CALL;
            plan->err_msg = ERRMSG_SYS_CALL;
	}
        else printf( "Dest(%s) IP(%s)\n", plan->dest_host, display_ip);
    }

    if( rc == RC_NORMAL)
    {
/*
printf( "ff:%d type:%d\n", plan->found_family, SOCK_DGRAM);
 */
        sock = socket( plan->found_family, SOCK_DGRAM, 0);
        if( sock == -1)
        {
            rc = ERR_SYS_CALL;
            plan->err_msg = ERRMSG_SYS_CALL;
	}
        else if( plan->found_family == AF_INET6)
        {
            sysrc = setsockopt( sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt_on, (sizeof opt_on));
            if( sysrc)
            {
                rc = ERR_SYS_CALL;
                plan->err_msg = ERRMSG_SYS_CALL;
	    }
	}
    }

    if( rc == RC_NORMAL)
    {
        msglen = strlen( plan->message);
/* 
printf( "sock:%d mlen:%d dlen:%d dtype:%d dport:%d msg(%s)\n", sock, msglen, destlen,
  ((struct sockaddr_in *)dest)->sin_family, ntohs(((struct sockaddr_in *)dest)->sin_port), plan->message);
 */
        sysrc = sendto( sock, plan->message, msglen, 0, dest, destlen);
        if( sysrc == -1)
        {
printf( "**Error(%d)** sendto() call failed, errno=%d\n", sysrc, errno);
            rc = ERR_SYS_CALL;
            plan->err_msg = ERRMSG_SYS_CALL;
	}
        else if( sysrc != msglen)
        {
printf( "**Error(%d/%d)** sendto() partial write, errno=%d\n", sysrc, destlen, errno);
            rc = ERR_SYS_CALL;
            plan->err_msg = ERRMSG_SYS_CALL;
	}
        else
        {
            printf( "Done... A %d character UDP message sent to %s (%s) port %d.\n",
              msglen, plan->dest_host, display_ip, plan->dest_port);
	}
    }

    /* --- */

    if( rc != RC_NORMAL)
    {
        printf( "Error, rc=%d\n", rc);
        if( plan) if( plan->err_msg) printf( "%s\n", plan->err_msg);
    }

    exit( rc);
}
