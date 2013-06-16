#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "err_ref.h"
#include "net-task-data.h"

/* --- */

int get_destination_ip( struct task_details *plan)

{
    int rc = RC_NORMAL, sysrc, errlen;
    struct addrinfo hints, *host_recs = 0, *walk, *match4 = 0, *match6 = 0, *next_host_rec = 0, *dup_list = 0;
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

    sysrc = inet_pton( AF_INET, plan->target_host, &plan->dest4.sin_addr);
    if( sysrc == 1)
    {
        plan->found_family = AF_INET;
        if( plan->debug >= DEBUG_MEDIUM) fprintf( stderr, "Found '%s' is a V4 IP address.\n", plan->target_host);
    }
    else if( sysrc) rc = ERR_OPT_CONFIG;
    else
    {
        sysrc = inet_pton( AF_INET6, plan->target_host, &plan->dest6.sin6_addr);
        if( sysrc == 1)
        {
            plan->found_family = AF_INET6;
            if( plan->debug >= DEBUG_MEDIUM) fprintf( stderr, "Found '%s' is a V6 IP address.\n", plan->target_host);
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

            sysrc = getaddrinfo( plan->target_host, 0, &hints, &host_recs);
            if( !sysrc)
            {
                if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "...getaddrinfo() call worked, scan the host records to find the one we want...\n");
                match4 = match6 = 0;

                for( walk = host_recs; rc == RC_NORMAL && walk; walk = walk->ai_next)
                {
                    next_host_rec = (struct addrinfo *) malloc( sizeof *next_host_rec);

                    if( !next_host_rec) rc = ERR_MALLOC_FAILED;
                    else
                    {
                        if( !dup_list) dup_list = next_host_rec;
                        else dup_list->ai_next = next_host_rec;

                        next_host_rec->ai_flags = walk->ai_flags;
                        next_host_rec->ai_family = walk->ai_family;
                        next_host_rec->ai_socktype = walk->ai_socktype;
                        next_host_rec->ai_protocol = walk->ai_protocol;
                        next_host_rec->ai_addrlen = walk->ai_addrlen;
                        next_host_rec->ai_addr = (struct sockaddr *) malloc( next_host_rec->ai_addrlen);
                        if( next_host_rec->ai_addr) memcpy( walk->ai_addr, next_host_rec->ai_addr,
                          next_host_rec->ai_addrlen);
                        next_host_rec->ai_canonname = strdup( walk->ai_canonname);
                        next_host_rec->ai_next = 0;

                        if( !next_host_rec->ai_canonname || !next_host_rec->ai_addr)
                          rc = ERR_MALLOC_FAILED;
		    }
		}

                if( rc == RC_NORMAL)
                {
                    freeaddrinfo( host_recs);
                    host_recs = dup_list;

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
    }

    if( !strcmp( plan->target_host, IPV6_LOOPBACK_ADDRESS)) plan->dest6.sin6_scope_id = SCOPE_LOOP;
    else if( IN6_IS_ADDR_LINKLOCAL( &plan->dest6.sin6_addr)) plan->dest6.sin6_scope_id = SCOPE_LINK;
    else if( IN6_IS_ADDR_SITELOCAL( &plan->dest6.sin6_addr)) plan->dest6.sin6_scope_id = SCOPE_SITE;
    else plan->dest6.sin6_scope_id = SCOPE_GLOBAL;

    if( rc == ERR_GETHOST_FAILED)
    {
        errlen = strlen( ERRMSG_GETHOST_FAILED) + strlen( plan->target_host);
        plan->err_msg = (char *) malloc( errlen);
        if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
        else snprintf( plan->err_msg, errlen, ERRMSG_GETHOST_FAILED, plan->target_host);
    }
    else if( rc == ERR_OPT_CONFIG)
    {
        errlen = strlen( ERRMSG_INET_PTON) + strlen( plan->target_host);
        plan->err_msg = (char *) malloc( errlen);
        if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
        else snprintf( plan->err_msg, errlen, ERRMSG_INET_PTON, plan->target_host);
    }

    return( rc);
}
