#include <stdlib.h>
#include <stdio.h>

#include "lock-n-run.h"
#include "parse_opt.h"
#include "err_ref.h"

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
    

    if( debug_level >= DEBUG_MEDIUM) print_parse_summary( extra_opts, opset, nflags);

    exit( rc);
}
