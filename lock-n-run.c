/* ??? Should stash away more useful error messages, in addition to RC settings */

#include <stdlib.h>
#include <stdio.h>

#include "lock-n-run.h"
#include "err_ref.h"
#include "cli-sub.h"

/* --- */

int main( int narg, char **opts)

{
    int rc = RC_NORMAL, max_wait = 0, lockmode_dec = 0, debug_level = 0;
    char *lockfile = 0, *run_user = 0, *run_group = 0, *psname = 0, *st = 0;
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
    };
    struct option_set *co = 0;
    struct word_chain *extra_opts = 0;
    struct word_list *comm_list = 0;
    int nflags = (sizeof opset) / (sizeof opset[0]);

    /* --- */

    if( narg < 2)
    {
        st = opts[ 0];
        if( *st == '.' && *(st + 1) == '/') st += 2;
        printf( MSG_SHOW_SYNTAX, st);
        exit( 1);
    }

    extra_opts = parse_command_options( &rc, opset, nflags, narg, opts);

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

    /* --- */
    
    if( rc == RC_NORMAL && debug_level >= DEBUG_MEDIUM) print_parse_summary( extra_opts, opset, nflags);

    /* --- */

    /* ---
     * - Now that we have the options stashed away neatly, check the settings
     * - for any that are set to "use the default" and resolve those
     * - references to usable values.  Also, apply some sanity checks to avoid
     * - unexpected and/or unpleasant side effects.
     */

    /* If the strings variables were set to an emtpy value, which has to be done explicitly, then
     * set an error and bail.
     */

    if( !*lockfile || !*psname || !*run_user || !*run_group) rc = ERR_OPT_CONFIG;

    if( rc == RC_NORMAL)
    {
        if( max_wait < 0) max_wait = 0;
    }

    if( rc == RC_NORMAL)
    {
        if( strcmp( run_user, USE_DEFAULT))
        {
            for( st = run_user; *st && rc == RC_NORMAL; st++)
              if( !isgraph( *st)) rc = ERR_INVALID_DATA;
	}
    }

    if( rc == RC_NORMAL)
    {
        if( strcmp( run_group, USE_DEFAULT))
        {
            for( st = run_group; *st && rc == RC_NORMAL; st++)
              if( !isgraph( *st)) rc = ERR_INVALID_DATA;
	}
    }

    if( rc == RC_NORMAL)
    {
        if( lockmode_dec >= 800 || lockmode_dec % 100 >= 80 || lockmode_dec % 10 >= 8)
        {
            rc = ERR_OPT_CONFIG;
	}
        else
        {
            lockmode = convert_to_mode( lockmode_dec);
            if( lockmode & (S_ISVTX | S_ISGID | S_ISUID))
            {
                rc = ERR_OPT_CONFIG;
	    }
	}
    }

    /* The lockfile check needs to be done after the "-user" and "-group" check in case they are used in the name */
    if( rc == RC_NORMAL)
    {
        if( !strcmp( lockfile, USE_DEFAULT))
        {
            ...set the lockfile to the default template with the appropriate username sub'd, use the run-user if diff...
	}
    }

    if( rc == RC_NORMAL)
    {
        ...what's to check with the command list?...
    }

    if( rc == RC_NORMAL)
    {
        if( !strcmp( psname, USE_DEFAULT))
        {
            ...copy the first word in the command line...
	}
        else
        {
            if( strlen( psname) > MAX_PROCESS_NAME_LEN)
            {
                *(psname + MAX_PROCESS_NAME_LEN - 1) = '\0';
                fprintf( stderr, "**Warning** Truncating displayed process name to %d characters, '%s'\n", 
                  MAX_PROCESS_NAME_LEN, psname);
	    }
            
            for( st = psname; *st && rc == RC_NORMAL; st++)
              if( !isprint( *st)) rc = ERR_INVALID_DATA;
	}
    }

    /* --- */


    if( rc != RC_NORMAL)
    {
        fprintf( stderr, "**Error(%d)** There will be a better error message here soon, rc=%d\n", rc, errno);
    }

    /* --- */

    exit( rc);
}
