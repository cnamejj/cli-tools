/*
 *
 * Todo
 * ----
 * - Scrub the environment variables in general if you want to be paranoid
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "lock-n-run.h"
#include "err_ref.h"
#include "cli-sub.h"

/* --- */

#define LOCK_DATA_LEN 128

/* --- */

void bail_on_timeout( int sig, siginfo_t *info, void *context)

{
    fprintf( stderr, "**Error** Timeout trying to establish lock.\n");

    exit( ERR_ALARMCLOCK);
}


/* --- */

char *build_filename_from_template( int *rc, char *template, char *comm, char *run_user, char *run_group)

{
    char time_buff[ 32], *result = 0;
    struct sub_list *patts = 0, *walk = 0;
    struct tm *local_now = 0;
    time_t now;

    /* --- */

    walk = patts = (struct sub_list *) malloc( sizeof *walk);
    if( !walk) *rc = ERR_MALLOC_FAILED;
    else
    {
        walk->from = "%{comm}";
        walk->to = comm;
        walk->next = (struct sub_list *) malloc( sizeof *walk);
        if( !walk->next) *rc = ERR_MALLOC_FAILED;
        else walk = walk->next;
    }

    if( *rc == RC_NORMAL)
    {
        walk->from = "%{user}";
        walk->to = run_user;
        walk->next = (struct sub_list *) malloc( sizeof *walk);
        if( !walk->next) *rc = ERR_MALLOC_FAILED;
        else walk = walk->next;
    }

    if( *rc == RC_NORMAL)
    {
        walk->from = "%{group}";
        walk->to = run_group;
        walk->next = (struct sub_list *) malloc( sizeof *walk);
        if( !walk->next) *rc = ERR_MALLOC_FAILED;
        else walk = walk->next;
    }

    if( *rc == RC_NORMAL)
    {
        time( &now);
        local_now = localtime( &now);
        if( !local_now) *rc = ERR_MALLOC_FAILED;
        else
        {
            time_buff[ 0] = '\0';
            if( !strftime( time_buff, (sizeof time_buff), TIME_FORMAT, local_now))
              *rc = ERR_SYS_CALL;
	}

        if( *rc == RC_NORMAL)
        {
            walk->from = "%{today}";
            walk->to = time_buff;
            walk->next = (struct sub_list *) malloc( sizeof *walk);
            if( !walk->next) *rc = ERR_MALLOC_FAILED;
            else walk = walk->next;
	}
    }

    if( *rc == RC_NORMAL)
    {
        walk->from = "%{date}";
        walk->to = time_buff;
        walk->next = 0;
    }

    if( *rc == RC_NORMAL) result = gsub_string( rc, template, patts);

    /* --- */

    return( result);
}


/* --- */

int main( int narg, char **opts)

{
    int rc = RC_NORMAL, max_wait = 0, lockmode_dec = 0, debug_level = 0, off, lf,
      sysrc, lock_data_len, show_help = 0, flags = O_WRONLY | O_CREAT;
    char *lockfile = 0, *run_user = 0, *run_group = 0, *psname = 0, *st = 0, *comm_name,
      *this_user = 0, *this_group = 0, lock_data[ LOCK_DATA_LEN], *errmsg = 0;
    struct option_set opset[] = {
      { OP_LOCKFILE, OP_TYPE_CHAR, OP_FL_BLANK, FL_LOCKFILE,   0, DEF_LOCKFILE, 0, 0 },
      { OP_LOCKFILE, OP_TYPE_CHAR, OP_FL_BLANK, FL_LOCKFILE_2, 0, DEF_LOCKFILE, 0, 0 },
      { OP_LOCKFILE, OP_TYPE_CHAR, OP_FL_BLANK, FL_LOCKFILE_3, 0, DEF_LOCKFILE, 0, 0 },
      { OP_LOCKFILE, OP_TYPE_CHAR, OP_FL_BLANK, FL_LOCKFILE_4, 0, DEF_LOCKFILE, 0, 0 },
      { OP_PSNAME,   OP_TYPE_CHAR, OP_FL_BLANK, FL_PSNAME,     0, DEF_PSNAME,   0, 0 },
      { OP_PSNAME,   OP_TYPE_CHAR, OP_FL_BLANK, FL_PSNAME_2,   0, DEF_PSNAME,   0, 0 },
      { OP_LOCKMODE, OP_TYPE_INT,  OP_FL_BLANK, FL_LOCKMODE,   0, DEF_LOCKMODE, 0, 0 },
      { OP_LOCKMODE, OP_TYPE_INT,  OP_FL_BLANK, FL_LOCKMODE_2, 0, DEF_LOCKMODE, 0, 0 },
      { OP_WAIT,     OP_TYPE_INT,  OP_FL_BLANK, FL_WAIT,       0, DEF_WAIT,     0, 0 },
      { OP_WAIT,     OP_TYPE_INT,  OP_FL_BLANK, FL_WAIT_2,     0, DEF_WAIT,     0, 0 },
      { OP_COMM,     OP_TYPE_LAST, OP_FL_BLANK, FL_COMM,       0, DEF_COMM,     0, 0 },
      { OP_COMM,     OP_TYPE_LAST, OP_FL_BLANK, FL_COMM_2,     0, DEF_COMM,     0, 0 },
      { OP_USER,     OP_TYPE_CHAR, OP_FL_BLANK, FL_USER,       0, DEF_USER,     0, 0 },
      { OP_USER,     OP_TYPE_CHAR, OP_FL_BLANK, FL_USER_2,     0, DEF_USER,     0, 0 },
      { OP_GROUP,    OP_TYPE_CHAR, OP_FL_BLANK, FL_GROUP,      0, DEF_GROUP,    0, 0 },
      { OP_GROUP,    OP_TYPE_CHAR, OP_FL_BLANK, FL_GROUP_2,    0, DEF_GROUP,    0, 0 },
      { OP_DEBUG,    OP_TYPE_INT,  OP_FL_BLANK, FL_DEBUG,      0, DEF_DEBUG,    0, 0 },
      { OP_HELP,     OP_TYPE_FLAG, OP_FL_BLANK, FL_HELP,       0, DEF_HELP,     0, 0 },
    };
    struct option_set *co = 0;
    struct word_chain *extra_opts = 0;
    struct word_list *comm_list = 0;
    struct sigaction sig, hold_sig;
    struct flock lock;
    mode_t lockmode, this_umask;
    pid_t pid, ppid;
    time_t now;
    int nflags = (sizeof opset) / (sizeof opset[0]);

    /* --- */

    if( narg < 2) show_help = 1;

    extra_opts = parse_command_options( &rc, opset, nflags, narg, opts);

    /* --- */

    if( rc == RC_NORMAL) this_user = get_username( &rc, geteuid());
    if( rc != RC_NORMAL) errmsg = ERR_GETUSERNAME_FAIL;

    if( rc == RC_NORMAL) this_group = get_groupname( &rc, getegid());
    if( rc != RC_NORMAL) errmsg = ERR_GETGROUPNAME_FAIL;

    if( rc == RC_NORMAL)
    {
        if( !this_user || !this_group) rc = ERR_MALLOC_FAILED;
        else if( !*this_user || !*this_group) rc = ERR_MALLOC_FAILED;
    }

    /* --- */
    
    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_DEBUG, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            debug_level = *((int *) co->parsed);
	}
    }
    
    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_LOCKFILE, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            lockfile = (char *) co->parsed;
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_PSNAME, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            psname = (char *) co->parsed;
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_LOCKMODE, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            lockmode_dec = *((int *) co->parsed);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_WAIT, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            max_wait = *((int *) co->parsed);
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_COMM, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            comm_list = (struct word_list *) co->parsed;
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_USER, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            run_user = (char *) co->parsed;
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_GROUP, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else
        {
            run_group = (char *) co->parsed;
	}
    }

    if( rc == RC_NORMAL)
    {
        co = get_matching_option( OP_HELP, opset, nflags);
        if( !co) rc = ERR_OPT_CONFIG;
        else if( co->flags & OP_FL_SET) show_help = 1;
    }

    /* --- */
    
    if( rc == RC_NORMAL && debug_level >= DEBUG_MEDIUM) print_parse_summary( extra_opts, opset, nflags);

    /* --- */

    /* ---
     * - Now that we have the options stashed away neatly, check the settings
     * - for any that are set to "use the default" and resolve those
     * - references to usable values.  Also, apply some sanity checks to avoid
     * - unexpected and/or unpleasant side effects.
     */

    /* If the strings variables were set to an empty value, which has to be done explicitly, then
     * set an error and bail.
     */

    if( !*lockfile || !*psname || !*run_user || !*run_group)
    {
        rc = ERR_INVALID_DATA;
        if( !*psname) errmsg = ERR_NULL_PSNAME;
        else if( !*run_user) errmsg = ERR_NULL_RUNUSER;
        else if( !*run_group) errmsg = ERR_NULL_RUNGROUP;
        else if( !*lockfile) errmsg = ERR_NULL_LOCKFILE;
    }

    if( rc == RC_NORMAL)
    {
        if( strcmp( run_user, USE_DEFAULT))
        {
            for( st = run_user; *st && rc == RC_NORMAL; st++) if( !isgraph( *st))
            {
                rc = ERR_INVALID_DATA;
                errmsg = ERR_INVALID_RUN_USER;
            }
	}
    }

    if( rc == RC_NORMAL)
    {
        if( strcmp( run_group, USE_DEFAULT))
        {
            for( st = run_group; *st && rc == RC_NORMAL; st++) if( !isgraph( *st))
            {
                rc = ERR_INVALID_DATA;
                errmsg = ERR_INVALID_RUN_GROUP;
	    }
	}
    }

    if( rc == RC_NORMAL)
    {
        if( lockmode_dec >= 800 || lockmode_dec % 100 >= 80 || lockmode_dec % 10 >= 8)
        {
            rc = ERR_INVALID_DATA;
            errmsg = ERR_INVALID_LOCK_MODE;
	}
        else
        {
            lockmode = convert_to_mode( lockmode_dec);
            if( lockmode & (S_ISVTX | S_ISGID | S_ISUID))
            {
                rc = ERR_INVALID_DATA;
                errmsg = ERR_SETX_LOCK_MODE;
	    }
	}
    }

    if( rc == RC_NORMAL)
    {
        if( !strcmp( lockfile, USE_DEFAULT))
        {
            lockfile = strdup( DEFAULT_LOCKFILE);
            if( !lockfile) rc = ERR_MALLOC_FAILED;
	}
        else if( strlen( lockfile) > MAXPATHLEN)
        {
            rc = ERR_INVALID_DATA;
            errmsg = ERR_MAX_PATH_LEN;
	}
        else
        {
            for( st = lockfile; *st && rc == RC_NORMAL; st++) if( !isprint( *st))
            {
                rc = ERR_INVALID_DATA;
                errmsg = ERR_INVALID_FILENAME;
	    }
	}
    }

    if( rc == RC_NORMAL)
    {
        if( comm_list->count < 1)
        {
            rc = ERR_INVALID_DATA;
            errmsg = ERR_NULL_COMMAND;
	}
        else
        {
            comm_name = strdup( comm_list->words[ 0]);
            if( !comm_name) rc = ERR_MALLOC_FAILED;
            else if( !strcmp( comm_name, USE_DEFAULT))
            {
                free( comm_name);
                free( comm_list->words[ 0]);
                comm_name = strdup( DEF_COMM);
                st = comm_list->words[ 0] = strdup( DEF_COMM);
                if( !comm_name || !st) rc = ERR_MALLOC_FAILED;
	    }
	}
    }

    /* Clean-up for "psname" has to come after clean-up for the command to be run */
    if( rc == RC_NORMAL)
    {
        if( !strcmp( psname, USE_DEFAULT))
        {
            psname = strdup( comm_name);
            if( !psname) rc = ERR_MALLOC_FAILED;
	}
        

        if( rc == RC_NORMAL)
        {
            if( strlen( psname) > NAME_MAX)
            {
                *(psname + NAME_MAX - 1) = '\0';
                fprintf( stderr, "**Warning** Truncating displayed process name to %d characters, '%s'\n", 
                  NAME_MAX, psname);
            }
            
            for( st = psname; *st && rc == RC_NORMAL; st++) if( !isprint( *st))
            {
                rc = ERR_INVALID_DATA;
                errmsg = ERR_INVALID_PSNAME;
	    }
	}

        /* If the psname isn't the same as the command, need to flip them */
	if( rc == RC_NORMAL && strcmp( psname, comm_name))
        {
            comm_list->words[ 0] = psname;
        }
    }

    if( rc == RC_NORMAL)
    {
        if( !strcmp( run_user, USE_DEFAULT))
        {
            free( run_user);
            run_user = this_user;
	}

        if( rc == RC_NORMAL) if( !strcmp( run_group, USE_DEFAULT))
        {
            free( run_group);
            run_group = this_group;
	}
    }

    /* ---
     * - The lockfile given might include placeholders like "%{user}" that need to be swapped
     * - for the real values.
     */

    if( rc == RC_NORMAL)
    {
        st = lockfile;
        lockfile = build_filename_from_template( &rc, st, comm_name, run_user, run_group);
        if( rc == RC_NORMAL) free( st);
    }

    /* --- */

    if( rc == RC_NORMAL) if( debug_level >= DEBUG_MEDIUM)
    {
        fprintf( stderr, "Here's what's to do...\n");
        fprintf( stderr, "Run-user.: '%s'\n", run_user);
        fprintf( stderr, "Run-group: '%s'\n", run_group);
        fprintf( stderr, "Command..:");
        for( off= 0; off <comm_list->count; off++) fprintf( stderr, " '%s'", comm_list->words[ off]);
        fprintf( stderr, "\n");
        fprintf( stderr, "Max-wait.: %d\n", max_wait);
        fprintf( stderr, "LockFile.: '%s'\n", lockfile);
        fprintf( stderr, "LockMode.: %x\n", lockmode);
        fprintf( stderr, "ProcName.: '%s'\n", psname);
    }

    /* --- */

    if( show_help)
    {
        st = opts[ 0];
        if( *st == '.' && *(st + 1) == '/') st += 2;
        printf( MSG_SHOW_SYNTAX, st);
        exit( 1);
    }


    /* --- */

    if( rc == RC_NORMAL) if( strcmp( run_group, this_group))
    {
        rc = switch_run_group( run_group);
        if( rc != RC_NORMAL) errmsg = ERR_SETGID_FAIL;
    }

    if( rc == RC_NORMAL) if( strcmp( run_user, this_user))
    {
        rc = switch_run_user( run_user);
        if( rc != RC_NORMAL) errmsg = ERR_SETUID_FAIL;
    }

    if( rc == RC_NORMAL)
    {
        this_umask = umask( 0);

        lf = open( lockfile, flags, lockmode);
        if( lf == -1)
        {
            rc = ERR_OPEN_FAILED;
            errmsg = ERR_LOCK_OPEN_FAIL;
	}
    }

    if( rc == RC_NORMAL)
    {
        lock.l_type = F_WRLCK;
        lock.l_whence = 0;
        lock.l_start = 0;

        if( max_wait > 0)
        {
            sig.sa_flags = SA_SIGINFO;
            sigemptyset( &sig.sa_mask);
            sig.sa_handler = 0;
            sig.sa_sigaction = bail_on_timeout;

            sysrc = sigaction( SIGALRM, &sig, &hold_sig);
            if( sysrc == -1) rc = ERR_SIGACTION_FAILED;

            if( rc == RC_NORMAL) (void) alarm( max_wait);
        }
    }

    if( rc == RC_NORMAL)
    {
        if( max_wait == 0) sysrc = fcntl( lf, F_SETLK, &lock);
        else
        {
            sysrc = fcntl( lf, F_SETLKW, &lock);
            if( max_wait > 0)
            {
                (void) alarm( 0);
                (void) sigaction( SIGALRM, &hold_sig, 0);
            }
        }

        if( sysrc == -1)
        {
            rc = ERR_CANT_LOCK;
            if( errno == EACCES || errno == EAGAIN) errmsg = ERR_LOCK_UNAVAILABLE;
            else if( errno == EINTR && max_wait > 0) errmsg = ERR_LOCK_TIMEOUT;
            else errmsg = ERR_LOCK_FAIL;
        }
    }

    if( rc == RC_NORMAL)
    {
        (void) umask( this_umask);

        pid = getpid();
        ppid = getppid();
        (void) time( &now);
        snprintf( lock_data, LOCK_DATA_LEN, "%d %d %ld\n", pid, ppid, now);

        lock_data_len = strlen( lock_data) + 1;
        sysrc = write( lf, lock_data, lock_data_len);
        if( sysrc != lock_data_len)
        {
            rc = ERR_WRITE_FAILED;
            errmsg = ERR_LOCK_WRITE_FAIL;
	}
    }

    if( rc == RC_NORMAL)
    {
        sysrc = setenv( ENV_IFS_VAR, ENV_IFS_VAL, DO_OVERWRITE);
        if( sysrc)
        {
            rc = ERR_SYS_CALL;
            errmsg = ERR_SETENV_FAIL;
	}
    }

    if( rc == RC_NORMAL)
    {
        (void) execv( comm_name, comm_list->words);
        rc = ERR_SYS_CALL;
        if( errno == EACCES || errno == ENOENT || errno == ENOEXEC || errno == ENOTDIR
          || errno == EPERM) errmsg = ERR_EXEC_BAD_COMMAND;
        else errmsg = ERR_EXEC_FAIL;
    }

    /* --- */


    if( rc != RC_NORMAL)
    {
        if( !errmsg) errmsg = cli_strerror( rc);

        if( !errmsg) errmsg = DEF_ERRMSG;
        else if( !*errmsg) errmsg = DEF_ERRMSG;

        fprintf( stderr, "**Error(%d)** %s, rc=%d\n", rc, errmsg, errno);
    }

    /* --- */

    exit( rc);
}
