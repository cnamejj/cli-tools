#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

#include "send-udp-message.h"
#include "cli-sub.h"
#include "err_ref.h"
#include "net-task-data.h"

/* --- */

int build_err_msg( struct task_details *plan, int err_code, char *err_template, char *err_component)

{
    int result, err_msg_len = 0;
    char *msg = 0;

    result = err_code;

    if( plan && err_template && err_component) if( *err_template)
    {
        err_msg_len = strlen( err_template) + strlen( err_component);
        msg = (char *) malloc( err_msg_len);
        if( !msg) result = ERR_MALLOC_FAILED;
        else
        {
            snprintf( msg, err_msg_len, err_template, err_code, err_component);
            plan->err_msg = msg;
	}
    }

    return( result);
}

/* --- */

struct task_details *figure_out_what_to_do( int *returncode, int narg, char **opts)

{
    int rc = RC_NORMAL, off, *int_p = 0, dest_set = 0;
    struct task_details *plan = 0;
    struct option_set opset[] = {
      { OP_DEST_BOTH, OP_TYPE_CHAR, OP_FL_BLANK, FL_DEST_BOTH,   0, DEF_DEST_BOTH, 0, 0 },
      { OP_DEST_BOTH, OP_TYPE_CHAR, OP_FL_BLANK, FL_DEST_BOTH_2, 0, DEF_DEST_BOTH, 0, 0 },
      { OP_DEST_BOTH, OP_TYPE_CHAR, OP_FL_BLANK, FL_DEST_BOTH_3, 0, DEF_DEST_BOTH, 0, 0 },
      { OP_MESSAGE,   OP_TYPE_CHAR, OP_FL_BLANK, FL_MESSAGE,     0, DEF_MESSAGE,   0, 0 },
      { OP_MESSAGE,   OP_TYPE_CHAR, OP_FL_BLANK, FL_MESSAGE_2,   0, DEF_MESSAGE,   0, 0 },
      { OP_MESSAGE,   OP_TYPE_CHAR, OP_FL_BLANK, FL_MESSAGE_3,   0, DEF_MESSAGE,   0, 0 },
      { OP_PORT,      OP_TYPE_INT,  OP_FL_BLANK, FL_PORT,        0, DEF_PORT,      0, 0 },
      { OP_PORT,      OP_TYPE_INT,  OP_FL_BLANK, FL_PORT_2,      0, DEF_PORT,      0, 0 },
      { OP_HOST,      OP_TYPE_CHAR, OP_FL_BLANK, FL_HOST,        0, DEF_HOST,      0, 0 },
      { OP_HOST,      OP_TYPE_CHAR, OP_FL_BLANK, FL_HOST_2,      0, DEF_HOST,      0, 0 },
      { OP_IPV4,      OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV4,        0, DEF_IPV4,      0, 0 },
      { OP_IPV4,      OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV4_2,      0, DEF_IPV4,      0, 0 },
      { OP_IPV6,      OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV6,        0, DEF_IPV6,      0, 0 },
      { OP_IPV6,      OP_TYPE_FLAG, OP_FL_BLANK, FL_IPV6_2,      0, DEF_IPV6,      0, 0 },
      { OP_DEBUG,     OP_TYPE_INT,  OP_FL_BLANK, FL_DEBUG,       0, DEF_DEBUG,     0, 0 },
      { OP_HELP,      OP_TYPE_FLAG, OP_FL_BLANK, FL_HELP,        0, DEF_HELP,      0, 0 },
      { OP_HEX,       OP_TYPE_FLAG, OP_FL_BLANK, FL_HEX,         0, DEF_HEX,       0, 0 },
      { OP_HEX,       OP_TYPE_FLAG, OP_FL_BLANK, FL_HEX_2,       0, DEF_HEX,       0, 0 },
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

        if( rc == RC_NORMAL)
        {
            co = get_matching_option( OP_DEBUG, opset, nflags);
            if( !co) rc = ERR_OPT_CONFIG;
            else plan->debug = *((int *) co->parsed);
        }

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
        else if( co->opt_num >= dest_set) plan->target_port = *((int *) co->parsed);
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_HOST, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->opt_num >= dest_set) plan->target_host = (char *) co->parsed;
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_IPV4, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->flags & OP_FL_SET) plan->use_ip |= DO_IPV4;
        else plan->use_ip &= ~DO_IPV4;
        ipv4 = co;
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_IPV6, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->flags & OP_FL_SET) plan->use_ip |= DO_IPV6;
        else plan->use_ip &= ~DO_IPV6;
        ipv6 = co;
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_HELP, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->flags & OP_FL_SET) plan->show_help = 1;
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_HEX, opset, nflags);
printf( "dbg:: hex option %d. %d %d %x/%x '%s' '%s' '%s'\n", co->opt_num, co->num, co->type, co->flags,
  OP_FL_SET, co->name, co->val, co->def);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->flags & OP_FL_SET) plan->msg_in_hex = 1;
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

int main( int narg, char **opts)

{
    int rc = RC_NORMAL, opt_on = 1, sysrc, sock, destlen, msglen, errlen;
    struct sockaddr *dest = 0;
    char *chrc = 0, *err_msg = 0, *st = 0;
    char display_ip[ IP_DISPLAY_SIZE];
    struct task_details *plan = 0;
    void *s_addr;

    /* --- */

    plan = figure_out_what_to_do( &rc, narg, opts);

    if( narg < 2) plan->show_help = 1;

    /* --- */

    if( rc == RC_NORMAL)
    {
        if( plan->debug >= DEBUG_LOW) fprintf( stderr, "\nPlan: host(%s) port(%d) ipv4(%d) ipv6(%d) msg(%s) hex(%d)\n",
          plan->target_host, plan->target_port, plan->use_ip & DO_IPV4, plan->use_ip & DO_IPV6, plan->message,
          plan->msg_in_hex);
    }

    if( plan->show_help)
    {
        st = opts[ 0];
        if( *st == '.' && *(st + 1) == '/') st += 2;
        printf( MSG_SHOW_SYNTAX, st);
	exit( 1);
    }

    /* --- */

    if( rc == RC_NORMAL)
    {
        rc = get_destination_ip( plan);

        if( rc == RC_NORMAL && !plan->found_family) rc = build_err_msg( plan, ERR_GETHOST_FAILED,
          ERRMSG_GETHOST_FAILED, plan->target_host);
    }

    if( rc == RC_NORMAL)
    {
        if( plan->found_family == AF_INET)
        {
            plan->dest4.sin_port = htons( plan->target_port);
            dest = (struct sockaddr *) &plan->dest4;
            destlen = (sizeof plan->dest4);
            s_addr = &plan->dest4.sin_addr;
	}
        else
        {
            plan->dest6.sin6_port = htons( plan->target_port);
            dest = (struct sockaddr *) &plan->dest6;
            destlen = (sizeof plan->dest6);
            s_addr = &plan->dest6.sin6_addr;
	}
        chrc = (char *) inet_ntop( plan->found_family, s_addr, display_ip, IP_DISPLAY_SIZE);

        if( !chrc)
        {
            rc = ERR_SYS_CALL;
            plan->err_msg = build_syscall_errmsg( "inet_ntop", errno);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
	}
        else if( plan->debug > DEBUG_LOW) fprintf( stderr, "Dest(%s) IP(%s)\n", plan->target_host,
          display_ip);
    }

    if( rc == RC_NORMAL)
    {
        if( plan->debug >= DEBUG_NOISY) fprintf( stderr, "ff:%d type:%d\n", plan->found_family, SOCK_DGRAM);

        sock = socket( plan->found_family, SOCK_DGRAM, 0);
        if( sock == -1)
        {
            rc = ERR_SYS_CALL;
            plan->err_msg = build_syscall_errmsg( "socket", errno);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
	}
        else if( plan->found_family == AF_INET6)
        {
            sysrc = setsockopt( sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt_on, (sizeof opt_on));
            if( sysrc)
            {
                rc = ERR_SYS_CALL;
                plan->err_msg = build_syscall_errmsg( "socket", errno);
                if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
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
            plan->err_msg = build_syscall_errmsg( "sendto", errno);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
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
              msglen, plan->target_host, display_ip, plan->target_port);
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
