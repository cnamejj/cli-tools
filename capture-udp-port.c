#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include "parse_opt.h"
#include "capture-udp-port.h"
#include "err_ref.h"
#include "net-task-data.h"

/* --- */

struct task_details *figure_out_what_to_do( int *returncode, int narg, char **opts)

{
    int rc = RC_NORMAL, off, *int_p = 0, server_set = 0;
    struct task_details *plan = 0;
    struct option_set opset[] = {
      { OP_SERVER,  OP_TYPE_CHAR, OP_FL_BLANK, FL_SERVER,    0, DEF_SERVER,  0, 0 },
      { OP_SERVER,  OP_TYPE_CHAR, OP_FL_BLANK, FL_SERVER_2,  0, DEF_SERVER,  0, 0 },
      { OP_USER,    OP_TYPE_CHAR, OP_FL_BLANK, FL_USER,      0, DEF_USER,    0, 0 },
      { OP_USER,    OP_TYPE_CHAR, OP_FL_BLANK, FL_USER_2,    0, DEF_USER,    0, 0 },
      { OP_PORT,    OP_TYPE_INT,  OP_FL_BLANK, FL_PORT,      0, DEF_PORT,    0, 0 },
      { OP_PORT,    OP_TYPE_INT,  OP_FL_BLANK, FL_PORT_2,    0, DEF_PORT,    0, 0 },
      { OP_HOST,    OP_TYPE_CHAR, OP_FL_BLANK, FL_HOST,      0, DEF_HOST,    0, 0 },
      { OP_HOST,    OP_TYPE_CHAR, OP_FL_BLANK, FL_HOST_2,    0, DEF_HOST,    0, 0 },
      { OP_IPV4,    OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV4,      0, DEF_IPV4,    0, 0 },
      { OP_IPV4,    OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV4_2,    0, DEF_IPV4,    0, 0 },
      { OP_IPV6,    OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV6,      0, DEF_IPV6,    0, 0 },
      { OP_IPV6,    OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV6_2,    0, DEF_IPV6,    0, 0 },
      { OP_LOGFILE, OP_TYPE_CHAR, OP_FL_BLANK, FL_LOGFILE,   0, DEF_LOGFILE, 0, 0 },
      { OP_LOGFILE, OP_TYPE_CHAR, OP_FL_BLANK, FL_LOGFILE_2, 0, DEF_LOGFILE, 0, 0 },
      { OP_MODE,    OP_TYPE_INT,  OP_FL_BLANK, FL_MODE,      0, DEF_MODE,    0, 0 },
      { OP_MODE,    OP_TYPE_INT,  OP_FL_BLANK, FL_MODE_2,    0, DEF_MODE,    0, 0 },
      { OP_DEBUG,   OP_TYPE_INT,  OP_FL_BLANK, FL_DEBUG,     0, DEF_DEBUG,   0, 0 },
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
        co = get_matching_option( OP_SERVER, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            server_set = co->opt_num;
            rc = parse_destination_value( plan, (char *) co->parsed);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_USER, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else plan->runuser = (char *) co->parsed;
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_PORT, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->opt_num >= server_set) plan->target_port = *((int *) co->parsed);
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_HOST, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->opt_num >= server_set) plan->target_host = (char *) co->parsed;
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
        co = get_matching_option( OP_LOGFILE, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else plan->logfile = (char *) co->parsed;
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_MODE, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else plan->logmode = *((int *) co->parsed);
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_DEBUG, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else plan->debug = *((int *) co->parsed);
    }

/* ...figure out how to deal with "--no-" options for host, port and server flags... */

    /* By default both IPV4 and IPV6 are disabled.  If neither option was selected via
     * command line flags, them turn the both on so the code will take whichever one it
     * finds.
     */

    if( !(plan->use_ip & (DO_IPV4 | DO_IPV6)))
    {
        if( ipv4->opt_num && ipv6->opt_num) rc = ERR_INVALID_DATA;
        else if( ipv4->opt_num) plan->use_ip |= DO_IPV6;
        else if( ipv6->opt_num) plan->use_ip |= DO_IPV4;
        else plan->use_ip |= (DO_IPV4 | DO_IPV6);
    }

    if( rc == ERR_OPT_CONFIG) plan->err_msg = "Unrecoverable internal configuration error, can't continue.";
    else if( rc == ERR_INVALID_DATA) plan->err_msg = "Both --no-ipv4 and --no-ipv6 specified, at least one needs to be allowed.";

    *returncode = rc;

    return( plan);
}

/* --- */

char *build_syscall_errmsg( char *syscall, int sysrc)

{
    int errlen = 0;
    char *errmsg = 0, *name = 0, *default = "unspecified system call";

    if( !syscall) name = default;
    else if( !*syscall) name = default;

    errlen = ERRMSG_SYSCALL2_FAILED + strlen( name) + INT_ERR_DISPLAY_LEN;
    errmsg = (char *) malloc( errlen);
    if( errmsg) snprintf( errmsg, errlen, ERRMSG_SYSCALL2_FAILED, name, sysrc);

    return( errmsg);
}

/* --- */

int switch_to_run_user( struct task_details *plan)

{
    int rc = RC_NORMAL;

    if( plan->runuser) if( *plan->runuser) if( strcmp( plan->runuser, ???NO_SWITCH_NEEDED???))
    {
...yeah, do that stuff to switch to the indicated user...
    }

    return( rc);
}

/* --- */

int open_logfile( int *log_fd, struct task_details *plan)

{
    int rc = RC_NORMAL;

...decode the value of "plan->logmode" and set umask...

...open the file in "plan->logfile"...

    return( rc);
}

/* --- */

int receive_udp_and_log( struct task_details *plan, int sock, int log_fd)

{
    int rc = RC_NORMAL, sysrc = 0;

    for(; rc == RC_NORMAL; )
    {
        sysrc = recvfrom( sock, buff, BUFFER_SIZE, 0, sender, sender_len);
        if( sysrc == -1)
        {
            sysrc = errno;
            if( sysrc == ENOMEM) rc = ERR_MALLOC_FAILED;
            else rc = ERR_SYS_CALL;
	}
        else
        {
...dump the record...
	}
    }

    /* --- */

    if( rc == ERR_SYS_CALL)
    {
        plan->err_msg = build_syscall_errmsg( "recvfrom", sysrc);
        if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
    }

    return( rc);
}

/* --- */

int main( int narg, char **opts)

{
    int rc = RC_NORMAL, opt_on = 1, sysrc, sock, errlen, listen_len, log_fd;
    struct sockaddr *listen = 0;
    char *chrc = 0, *err_msg = 0;
    char display_ip[ IP_DISPLAY_SIZE];
    struct task_details *plan = 0;
    void *s_addr;

    /* --- */

    plan = figure_out_what_to_do( &rc, narg, opts);

    if( rc == RC_NORMAL)
    {
        if( plan->debug >= DEBUG_LOW) fprintf( stderr, 
          "\nPlan: server(%s) port(%d) ipv4(%d) ipv6(%d) user(%s) log(%s) mode(%d)\n",
          plan->target_host, plan->target_port, plan->use_ip & DO_IPV4, plan->use_ip & DO_IPV6,
          plan->runuser, plan->logfile, plan->logmode);
    }

    /* --- */

    if( rc == RC_NORMAL)
    {
        rc = get_destination_ip( plan);

        if( rc == RC_NORMAL && !plan->found_family)
        {
            rc = ERR_GETHOST_FAILED;
            errlen = strlen( ERRMSG_GETHOST_FAILED) + strlen( plan->target_host);
            plan->err_msg = (char *) malloc( errlen);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
            else snprintf( plan->err_msg, errlen, ERRMSG_GETHOST_FAILED, plan->target_host);
	}
    }

    if( rc == RC_NORMAL)
    {
        if( plan->found_family == AF_INET)
        {
            plan->dest4.sin_port = htons( plan->target_port);
            listen = (struct sockaddr *) &plan->dest4;
            listen_len = (sizeof plan->dest4);
            s_addr = &plan->dest4.sin_addr;
	}
        else
        {
            plan->dest6.sin6_port = htons( plan->target_port);
            listen = (struct sockaddr *) &plan->dest6;
            listen_len = (sizeof plan->dest6);
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
        else if( plan->debug > DEBUG_LOW) fprintf( stderr, 
          "Server(%s) IP(%s)\n", plan->target_host, display_ip);
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
#ifndef __APPLE__
            if( !sysrc) sysrc = setsockopt( sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &opt_on, (sizeof opt_on));
            if( !sysrc) sysrc = setsockopt( sock, IPPROTO_IPV6, IPV6_RECVHOPOPTS, &opt_on, (sizeof opt_on));
            if( !sysrc) sysrc = setsockopt( sock, IPPROTO_IPV6, IPV6_RECVRTHDR, &opt_on, (sizeof opt_on));
            if( !sysrc) sysrc = setsockopt( sock, IPPROTO_IPV6, IPV6_RECVDSTOPTS, &opt_on, (sizeof opt_on));
#endif
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

    sysrc = bind( sock, listen, listen_len);
    if( sysrc == -1)
    {
        rc = ERR_SYS_CALL;
        errlen = strlen( ERRMSG_BIND_FAILED) + INT_ERR_DISPLAY_LEN;
        plan->err_msg = (char *) malloc( errlen);
        if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
        else snprintf( plan->err_msg, errlen, ERRMSG_BIND_FAILED, errno);
    }

    if( rc == RC_NORMAL) rc = switch_to_run_user( plan);

    if( rc == RC_NORMAL) rc = open_logfile( &log_fd, plan);

    if( rc == RC_NORMAL) rc = receive_udp_and_log( plan, socket, log_fd);

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
