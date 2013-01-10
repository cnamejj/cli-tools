/* ??? need to make sure we don't ignore unrecognized flags, should print an error and exit */
/* ??? also, the first IP logged when listening on an ivp6 address appears to be wrong, subsequent recvfrom() calls work normally, might be a bug? create a simple program to demonstrate */
/* ??? figure out how to deal with "--no-" options for host, port and server flags, not sure what those would mean? */
/* ??? right now,  sysrc = open( plan->logfile, LOG_OPEN_FLAGS, mode);   has hardcoded LOG_OPEN_FLAGS but that should be configurable via command line options */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "parse_opt.h"
#include "capture-udp-port.h"
#include "err_ref.h"
#include "net-task-data.h"

/* --- */

struct task_details *figure_out_what_to_do( int *, int, char **);
char *build_syscall_errmsg( char *, int);
int switch_user_and_group( struct task_details *);
int switch_run_group( struct task_details *);
int switch_run_user( struct task_details *);
int open_logfile( int *, struct task_details *);
int receive_udp_and_log( struct task_details *, int, int);
mode_t convert_to_mode( int);

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
      { OP_GROUP,   OP_TYPE_CHAR, OP_FL_BLANK, FL_GROUP,     0, DEF_GROUP,   0, 0 },
      { OP_GROUP,   OP_TYPE_CHAR, OP_FL_BLANK, FL_GROUP_2,   0, DEF_GROUP,   0, 0 },
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

        /* Need to pull this one up (most options are parsed later) */

        if( rc == RC_NORMAL)
        {
            co = get_matching_option( OP_DEBUG, opset, nflags);
            if( !co) rc = ERR_OPT_CONFIG;
            else
            {
                plan->debug = *((int *) co->parsed);
                if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "Opt #%d, debug '%d'\n",
                  co->opt_num, *((int *) co->parsed));
            }
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

            /* Print out settings just for options included on the command line */
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
                        fprintf( stderr, "%d ", *int_p);
                    }
                    else if( co->type == OP_TYPE_CHAR) fprintf( stderr, "(%s) ", (char *) co->parsed);
                    else if( co->type == OP_TYPE_FLOAT) fprintf( stderr, "%f ", *((float *) co->parsed));
                    else fprintf( stderr, "/?/ ");
                    fprintf( stderr, "(%s) (%s) ", co->name, co->val);
                    fprintf( stderr, "\n");
                }
            }

            /* Print out all options settings, includes defaults for unspecified options */
            fprintf( stderr, "Seq Num Typ Fl Opt\n");

            for( off= 0; off < nflags; off++)
            {
                co = opset + off;
                fprintf( stderr, "%2d. %3d %3d %2x ", off + 1, co->num, co->type, co->flags);
                fprintf( stderr, "%3d ", co->opt_num);
                if( co->type == OP_TYPE_INT || co->type == OP_TYPE_FLAG)
                {
                    int_p = (int *) co->parsed;
                    fprintf( stderr, "%d ", *int_p);
                }
                else if( co->type == OP_TYPE_CHAR) fprintf( stderr, "(%s) ", (char *) co->parsed);
                else if( co->type == OP_TYPE_FLOAT) fprintf( stderr, "%f ", *((float *) co->parsed));
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
            if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "Opt #%d, server '%s'\n",
              co->opt_num, (char *) co->parsed);
            server_set = co->opt_num;
            rc = parse_destination_value( plan, (char *) co->parsed);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_USER, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            plan->runuser = (char *) co->parsed;
            if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "Opt #%d, run-user '%s'\n",
              co->opt_num, (char *) co->parsed);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_GROUP, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            plan->rungroup = (char *) co->parsed;
            if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "Opt #%d, run-group '%s'\n",
              co->opt_num, (char *) co->parsed);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_PORT, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            if( co->opt_num >= server_set) plan->target_port = *((int *) co->parsed);
            if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "Opt #%d, port '%d'\n",
              co->opt_num, *((int *) co->parsed));
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_HOST, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            if( co->opt_num >= server_set) plan->target_host = (char *) co->parsed;
            if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "Opt #%d, host '%s'\n",
              co->opt_num, (char *) co->parsed);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_IPV4, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            if( co->flags == OP_FL_SET) plan->use_ip |= DO_IPV4;
            else plan->use_ip &= ~DO_IPV4;
            ipv4 = co;
            if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "Opt #%d, ipv6=%x, use-ip=%x\n",
              co->opt_num, co->flags, plan->use_ip);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_IPV6, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            if( co->flags == OP_FL_SET) plan->use_ip |= DO_IPV6;
            else plan->use_ip &= ~DO_IPV6;
            ipv6 = co;
            if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "Opt #%d, ipv6=%x, use-ip=%x\n",
              co->opt_num, co->flags, plan->use_ip);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_LOGFILE, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            plan->logfile = (char *) co->parsed;
            if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "Opt #%d, logfile '%s'\n",
              co->opt_num, (char *) co->parsed);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_MODE, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            plan->logmode = *((int *) co->parsed);
            if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "Opt #%d, logmode '%d'\n",
              co->opt_num, *((int *) co->parsed));
	}
    }

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

        if( plan->debug >= DEBUG_HIGH) fprintf( stderr, "IP check, ipv4=%d, ipv6=%d, use_ip=%x\n",
          ipv4->opt_num, ipv6->opt_num, plan->use_ip);
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
    char *errmsg = 0, *name = 0, *def_name = "unspecified system call";

    if( !syscall) name = def_name;
    else if( !*syscall) name = def_name;
    else name = syscall;

    errlen = strlen( ERRMSG_SYSCALL2_FAILED) + strlen( name) + INT_ERR_DISPLAY_LEN;
    errmsg = (char *) malloc( errlen);
    if( errmsg) snprintf( errmsg, errlen, ERRMSG_SYSCALL2_FAILED, name, sysrc);

    return( errmsg);
}

/* --- */

int switch_user_and_group( struct task_details *plan)

{
    int rc = RC_NORMAL;

    rc = switch_run_group( plan);
    if( rc == RC_NORMAL) rc = switch_run_user( plan);

    return( rc);
}

/* --- */

int switch_run_group( struct task_details *plan)

{
    int rc = RC_NORMAL, sysrc, errlen;
    char *my_group = 0, *my_egroup = 0;
    struct group *gr = 0;
    uid_t my_gid, my_egid;

    if( plan->rungroup) if( *plan->rungroup) if( strcmp( plan->rungroup, NO_SWITCH_NEEDED))
    {
        my_gid = getgid();
        my_egid = getegid();

        errno = 0;
        gr = getgrgid( my_gid);
        if( !gr)
        {
            sysrc = errno;
            rc = ERR_SYS_CALL;
            errlen = strlen( ERRMSG_CURR_GRLOOKUP_FAILED) + INT_ERR_DISPLAY_LEN * 2;
            plan->err_msg = (char *) malloc( errlen);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
            else snprintf( plan->err_msg, errlen, ERRMSG_CURR_GRLOOKUP_FAILED, my_gid, sysrc);
	}
        else
        {
            my_group = strdup( gr->gr_name);
            if( !my_group) rc = ERR_MALLOC_FAILED;
	}
        
        if( rc == RC_NORMAL)
        {
            errno = 0;
            gr = getgrgid( my_egid);
            if( !gr)
            {
                sysrc = errno;
                rc = ERR_SYS_CALL;
                errlen = strlen( ERRMSG_CURR_GRLOOKUP_FAILED) + INT_ERR_DISPLAY_LEN;
                plan->err_msg = (char *) malloc( errlen);
                if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
                else snprintf( plan->err_msg, errlen, ERRMSG_CURR_GRLOOKUP_FAILED, my_gid, sysrc);
            }
            else
            {
                my_egroup = strdup( gr->gr_name);
                if( !my_egroup) rc = ERR_MALLOC_FAILED;
	    }
	}

        if( rc == RC_NORMAL) if( plan->debug >= DEBUG_LOW)
        {
            fprintf( stderr, "Current group is %s, gid #%d.", my_group, my_gid);
            if( my_egid != my_gid) fprintf( stderr, " Effective group %s, gid #%d.\n", my_egroup, my_egid);
            else fprintf( stderr, "\n");
	}

        if( rc == RC_NORMAL)
        {
            errno = 0;
            gr = getgrnam( plan->rungroup);
            if( !gr && errno)
            {
                plan->err_msg = build_syscall_errmsg( "getgrnam", sysrc);
                if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
	    }
            else if( !gr)
            {
                errlen = strlen( ERRMSG_NO_SUCH_GROUP) + strlen( plan->rungroup);
                plan->err_msg = (char *) malloc( errlen);
                if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
                else snprintf( plan->err_msg, errlen, ERRMSG_NO_SUCH_GROUP, plan->rungroup);
	    }
            else
            {
                sysrc = setgid( gr->gr_gid);
                if( sysrc)
                {
                    plan->err_msg = build_syscall_errmsg( "setgid", sysrc);
                    if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
		}
                else if( plan->debug >= DEBUG_LOW) fprintf( stderr, "Switched group to %s, gid #%d.\n",
                  plan->rungroup, gr->gr_gid);
	    }
	}
    }

    if( my_group) free( my_group);
    if( my_egroup) free( my_egroup);

    return( rc);
}

/* --- */

int switch_run_user( struct task_details *plan)

{
    int rc = RC_NORMAL, sysrc, errlen;
    char *my_name = 0, *my_ename = 0;
    struct passwd *pw = 0;
    uid_t my_uid, my_euid;

    if( plan->runuser) if( *plan->runuser) if( strcmp( plan->runuser, NO_SWITCH_NEEDED))
    {
        my_uid = getuid();
        my_euid = geteuid();

        errno = 0;
        pw = getpwuid( my_uid);
        if( !pw)
        {
            sysrc = errno;
            rc = ERR_SYS_CALL;
            errlen = strlen( ERRMSG_CURR_PWLOOKUP_FAILED) + INT_ERR_DISPLAY_LEN * 2;
            plan->err_msg = (char *) malloc( errlen);
            if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
            else snprintf( plan->err_msg, errlen, ERRMSG_CURR_PWLOOKUP_FAILED, my_uid, sysrc);
	}
        else
        {
            my_name = strdup( pw->pw_name);
            if( !my_name) rc = ERR_MALLOC_FAILED;
	}
        
        if( rc == RC_NORMAL)
        {
            errno = 0;
            pw = getpwuid( my_euid);
            if( !pw)
            {
                sysrc = errno;
                rc = ERR_SYS_CALL;
                errlen = strlen( ERRMSG_CURR_PWLOOKUP_FAILED) + INT_ERR_DISPLAY_LEN;
                plan->err_msg = (char *) malloc( errlen);
                if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
                else snprintf( plan->err_msg, errlen, ERRMSG_CURR_PWLOOKUP_FAILED, my_euid, sysrc);
            }
            else
            {
                my_ename = strdup( pw->pw_name);
                if( !my_ename) rc = ERR_MALLOC_FAILED;
	    }
	}

        if( rc == RC_NORMAL) if( plan->debug >= DEBUG_LOW)
        {
            fprintf( stderr, "Currently running as %s, uid #%d.", my_name, my_uid);
            if( my_euid != my_uid) fprintf( stderr, " Effective id %s, uid #%d.\n", my_ename, my_euid);
            else fprintf( stderr, "\n");
	}

        if( rc == RC_NORMAL)
        {
            errno = 0;
            pw = getpwnam( plan->runuser);
            if( !pw && errno)
            {
                plan->err_msg = build_syscall_errmsg( "getpwnam", sysrc);
                if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
	    }
            else if( !pw)
            {
                errlen = strlen( ERRMSG_NO_SUCH_USER) + strlen( plan->runuser);
                plan->err_msg = (char *) malloc( errlen);
                if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
                else snprintf( plan->err_msg, errlen, ERRMSG_NO_SUCH_USER, plan->runuser);
	    }
            else
            {
                sysrc = setuid( pw->pw_uid);
                if( sysrc)
                {
                    plan->err_msg = build_syscall_errmsg( "setuid", sysrc);
                    if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
		}
                else if( plan->debug >= DEBUG_LOW) fprintf( stderr, "Switched user to %s, uid #%d.\n",
                  plan->runuser, pw->pw_uid);
	    }
	}
    }

    if( my_name) free( my_name);
    if( my_ename) free( my_ename);

    return( rc);
}

/* --- */

mode_t convert_to_mode( int dec_mode)

{
    int subset;
    mode_t mode;

    mode = 0;

    subset = dec_mode % 10;
    if( subset & SUBSET_EXEC) mode |= S_IXOTH;
    if( subset & SUBSET_WRITE) mode |= S_IWOTH;
    if( subset & SUBSET_READ) mode |= S_IROTH;

    subset = (dec_mode / 10) % 10;
    if( subset & SUBSET_EXEC) mode |= S_IXGRP;
    if( subset & SUBSET_WRITE) mode |= S_IWGRP;
    if( subset & SUBSET_READ) mode |= S_IRGRP;
    
    subset = (dec_mode / 100) % 10;
    if( subset & SUBSET_EXEC) mode |= S_IXUSR;
    if( subset & SUBSET_WRITE) mode |= S_IWUSR;
    if( subset & SUBSET_READ) mode |= S_IRUSR;
    
    subset = (dec_mode / 1000) % 10;
    if( subset & SUBSET_EXEC) mode |= S_ISVTX;
    if( subset & SUBSET_WRITE) mode |= S_ISGID;
    if( subset & SUBSET_READ) mode |= S_ISUID;

    return( mode);
}

/* --- */

int open_logfile( int *log_fd, struct task_details *plan)

{
    int rc = RC_NORMAL, sysrc, errlen;
    mode_t mode, save_umask;

    save_umask = umask( 0);

    mode = convert_to_mode( plan->logmode);

    sysrc = open( plan->logfile, LOG_OPEN_FLAGS, mode);
    if( sysrc > 0) *log_fd = sysrc;
    else
    {
        sysrc = errno;
        rc = ERR_OPEN_FAILED;
        errlen = strlen( ERRMSG_OPEN_FAILED) + strlen( plan->logfile)
          + INT_ERR_DISPLAY_LEN;
        plan->err_msg = (char *) malloc( errlen);
        if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
        else snprintf( plan->err_msg, errlen, ERRMSG_OPEN_FAILED, plan->logfile, sysrc);
    }

    if( plan->debug >= DEBUG_LOW) fprintf( stderr, "Opened log file '%s', mode=%x (%d), fd=%d, err=%d\n",
      plan->logfile, mode, plan->logmode, sysrc, errno);

    (void) umask( save_umask);

    return( rc);
}

/* --- */

int receive_udp_and_log( struct task_details *plan, int sock, int log_fd)

{
    int rc = RC_NORMAL, sysrc = 0, inlen, outlen, errlen, meta_len, prefix_len = 0;
    char buff[ BUFFER_SIZE], logbuff[ LOG_BUFFER_SIZE], display_time[ TIME_DISPLAY_SIZE],
      display_ip[ IP_DISPLAY_SIZE], display_len[ LENGTH_DISPLAY_SIZE];
    char *outbuff = 0, *inbuff = 0, *last = 0, *prefix_buff = 0, *dis_ip = 0;
    void *s_addr = 0;
    struct sockaddr sender;
    struct sockaddr_in6 *sender6 = 0;
    struct sockaddr_in *sender4 = 0;
    struct tm this_time;
    socklen_t sender_len;
    time_t now;

    sender6 = (struct sockaddr_in6 *) &sender;
    sender4 = (struct sockaddr_in *) &sender;
    if( plan->found_family == AF_INET6) s_addr = &sender6->sin6_addr;
    else s_addr = &sender4->sin_addr;

    sender_len = (sizeof sender);

    for(; rc == RC_NORMAL; )
    {
        inlen = sysrc = recvfrom( sock, buff, BUFFER_SIZE, 0, &sender, &sender_len);
        if( sysrc == -1)
        {
            sysrc = errno;
            if( sysrc == ENOMEM) rc = ERR_MALLOC_FAILED;
            else
            {
                rc = ERR_SYS_CALL;
                plan->err_msg = build_syscall_errmsg( "recvfrom", sysrc);
                if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
	    }
	}
        else
        {
            time( &now);
            localtime_r( &now, &this_time);
            strftime( display_time, (sizeof display_time), TIME_DISPLAY_FORMAT, &this_time);

            dis_ip = (char *) inet_ntop( plan->found_family, s_addr, display_ip, (sizeof display_ip));
            if( !dis_ip) dis_ip = UNKNOWN_IP;
            else if( !*dis_ip) dis_ip = UNKNOWN_IP;

            if( plan->debug >= DEBUG_MEDIUM) fprintf( stderr, "Recv: %s %s %d\n", display_time, dis_ip, inlen);

            snprintf( display_len, (sizeof display_len), LENGTH_DISPLAY_FORMAT,	inlen);

            meta_len = strlen( display_time) + strlen( dis_ip) + strlen( display_len) + 1;
            if( meta_len > prefix_len)
            {
                if( prefix_buff) free( prefix_buff);
                prefix_len = meta_len;
                prefix_buff = (char *) malloc( prefix_len);
                if( !prefix_buff) rc = ERR_MALLOC_FAILED;
 	    }

            if( rc == RC_NORMAL)
            {
                snprintf( prefix_buff, prefix_len, "%s%s%s", display_time, dis_ip, display_len);

                outlen = prefix_len - 1;
                sysrc = write( log_fd, prefix_buff, outlen);

                if( sysrc == outlen)
                {
                    inbuff = buff;
                    outbuff = logbuff;

                    for( last = inbuff + inlen; inbuff < last; inbuff++)
                    {
                        snprintf( outbuff, 3, "%02x", *inbuff);
                        outbuff += 2;
	            }

                    *outbuff = '\n';

                    outlen = (inlen * 2) + 1;
                    sysrc = write( log_fd, logbuff, outlen);
		}

                if( sysrc < 0)
                {
                    sysrc = errno;
                    rc = ERR_WRITE_FAILED;
                    plan->err_msg = build_syscall_errmsg( "write", sysrc);
                    if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
                }
                else if( sysrc != outlen)
                {
                    rc = ERR_WRITE_FAILED;
                    errlen = strlen( ERRMSG_WRITE_PARTIAL) + INT_ERR_DISPLAY_LEN * 2;
                    plan->err_msg = (char *) malloc( errlen);
                    if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
                    else snprintf( plan->err_msg, errlen, ERRMSG_WRITE_PARTIAL,
                      outlen, sysrc);
                }
                else if( plan->debug >= DEBUG_MEDIUM) fprintf( stderr, "Wrote all data to log file\n");
	    }
	}
    }

    /* --- */

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
        else if( plan->debug >= DEBUG_LOW) fprintf( stderr, 
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

    if( rc == RC_NORMAL) rc = switch_user_and_group( plan);

    if( rc == RC_NORMAL) rc = open_logfile( &log_fd, plan);

    if( rc == RC_NORMAL) rc = receive_udp_and_log( plan, sock, log_fd);

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
