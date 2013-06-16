/* -----
 * Actor
 * --
 *
 * Map the a placeholder "command" to another executable and associated
 * meta-data and execute it if allowed.
 *
 * Todo
 * ----
 * - Should scrub the environment vars at some point, to be paranoid.
 */

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "cli-sub.h"
#include "err_ref.h"
#include "actor.h"

/* --- */

struct syslog_opts *get_syslog_options( xmlNodePtr head, int *rc)

{
    int local_rc = RC_NORMAL;
    char *st = 0;
    struct syslog_opts *sop = 0;
    xmlNodePtr slog = 0;

    sop = (struct syslog_opts *) malloc( (sizeof *sop));
    if( !sop) *rc = ERR_MALLOC_FAILED;
    else
    {
        sop->val_level = sop->val_facility = 0;
        sop->log_level = sop->log_facility = 0;

        slog = get_node_match( head, SYSLOG_TAG, GNM_LAST);
        if( slog)
        {
            sop->val_level = dup_attrib_value( rc, slog, IS_LOG_LEVEL);
            if( *rc == RC_NORMAL || *rc == ERR_NO_MATCH)
            {
                sop->val_facility = dup_attrib_value( rc, slog, IS_LOG_FACILITY);
		if( *rc == ERR_NO_MATCH) *rc = RC_NORMAL;
	    }
	}
        else *rc = ERR_USE_DEFAULT;
    }

      
    if( *rc == RC_NORMAL)
    {
        st = sop->val_level;
        if( !st) local_rc = ERR_USE_DEFAULT;
        else if( !*st) local_rc = ERR_USE_DEFAULT;
        else local_rc = RC_NORMAL;

        if( local_rc != RC_NORMAL) sop->log_level = DEF_LOG_LEVEL;
        else
        {
            for( ; *st; st++) *st = tolower( *st);

            st = sop->val_level;
            if( !strcmp( st, IS_LOG_LEVEL_NONE)) sop->log_level = LOG_NONE;
            else if( !strcmp( st, IS_LOG_LEVEL_DEBUG)) sop->log_level = LOG_DEBUG;
            else if( !strcmp( st, IS_LOG_LEVEL_INFO)) sop->log_level = LOG_INFO;
            else if( !strcmp( st, IS_LOG_LEVEL_NOTICE)) sop->log_level = LOG_NOTICE;
            else if( !strcmp( st, IS_LOG_LEVEL_WARNING)) sop->log_level = LOG_WARNING;
            else if( !strcmp( st, IS_LOG_LEVEL_ERROR)) sop->log_level = LOG_ERR;
            else if( !strcmp( st, IS_LOG_LEVEL_ERR)) sop->log_level = LOG_ERR;
            else *rc = ERR_INVALID_DATA;
	}
    }

    if( *rc == RC_NORMAL)
    {
        st = sop->val_facility;
        if( !st) local_rc = ERR_USE_DEFAULT;
        else if( !*st) local_rc = ERR_USE_DEFAULT;
        else local_rc = RC_NORMAL;

        if( local_rc != RC_NORMAL) sop->log_facility = DEF_LOG_FACILITY;
        else
        {
            for( ; *st; st++) *st = tolower( *st);

            st = sop->val_facility;
            if( !strcmp( st, IS_LOG_FACIL_AUTH)) sop->log_facility = LOG_AUTHPRIV;
            else if( !strcmp( st, IS_LOG_FACIL_AUTHPRIV)) sop->log_facility = LOG_AUTHPRIV;
            else if( !strcmp( st, IS_LOG_FACIL_LOCAL0)) sop->log_facility = LOG_LOCAL0;
            else if( !strcmp( st, IS_LOG_FACIL_LOCAL1)) sop->log_facility = LOG_LOCAL1;
            else if( !strcmp( st, IS_LOG_FACIL_LOCAL2)) sop->log_facility = LOG_LOCAL2;
            else if( !strcmp( st, IS_LOG_FACIL_LOCAL3)) sop->log_facility = LOG_LOCAL3;
            else if( !strcmp( st, IS_LOG_FACIL_LOCAL4)) sop->log_facility = LOG_LOCAL4;
            else if( !strcmp( st, IS_LOG_FACIL_LOCAL5)) sop->log_facility = LOG_LOCAL5;
            else if( !strcmp( st, IS_LOG_FACIL_LOCAL6)) sop->log_facility = LOG_LOCAL6;
            else if( !strcmp( st, IS_LOG_FACIL_LOCAL7)) sop->log_facility = LOG_LOCAL7;
            else if( !strcmp( st, IS_LOG_FACIL_USER)) sop->log_facility = LOG_AUTHPRIV;
            else *rc = ERR_INVALID_DATA;
	}
    }

    return( sop);
}

/* --- */

struct comm_alias *get_alias_rule( int *rc, xmlNodePtr node, char *user, char *alias)

{
    int psname_len;
    char *st = 0;
    struct comm_alias *comm_info = 0;
    xmlNodePtr curr, allow;

    *rc = RC_NORMAL;

    comm_info = (struct comm_alias *) malloc( (sizeof *comm_info));
    if( !comm_info) *rc = ERR_MALLOC_FAILED;
    else
    {
        comm_info->runuser = comm_info->rungroup = comm_info->command = comm_info->psname = 0;

        curr = search_node_list( node->children, ALIAS_TAG, IS_ALIAS, alias);
        if( !curr) *rc = ERR_NO_MATCH;
        else
        {
            allow = search_node_list( curr->children, ALLOW_TAG, IS_USER, user);
            if( !allow) *rc = ERR_DISALLOW;
            else
            {
                comm_info->runuser = dup_attrib_value( rc, allow, IS_RUN_USER);
                comm_info->rungroup = dup_attrib_value( rc, allow, IS_RUN_GROUP);
                comm_info->psname = dup_attrib_value( rc, allow, IS_PS_NAME);
                comm_info->command = dup_attrib_value( rc, allow, IS_COMMAND);
                if( !comm_info->runuser) comm_info->runuser = dup_attrib_value( rc, curr, IS_RUN_USER);
                if( !comm_info->rungroup) comm_info->rungroup = dup_attrib_value( rc, curr, IS_RUN_GROUP);
                if( !comm_info->psname) comm_info->psname = dup_attrib_value( rc, curr, IS_PS_NAME);
                if( !comm_info->command) comm_info->command = dup_attrib_value( rc, curr, IS_COMMAND);

                /* Aside from a malloc() failure, none of the other possible errors from the
                 * dup_attrib_value() call matter in this section of the program.
                 */
                if( *rc != ERR_MALLOC_FAILED)
                {
                    *rc = RC_NORMAL;

                    if( comm_info->command)
                    {
                        if( comm_info->psname) if( !*comm_info->psname) comm_info->psname = 0;
                        if( !comm_info->psname)
                        {
                            st = index( comm_info->command, BLANK);
                            if( !st) psname_len = strlen( comm_info->command) + 1;
                            else psname_len = st - comm_info->command + 1;

                            if( psname_len)
                            {
                                st = (char *) malloc( psname_len);
                                if( !st) *rc = ERR_MALLOC_FAILED;
                                else
                                {
                                    memcpy( st, comm_info->command, psname_len - 1);
                                    *(st + psname_len - 1) = EOS;
                                    comm_info->psname = st;
				}
			    }
			}
                    }
		}

                if( *rc == RC_NORMAL)
                {
                    if( !comm_info->runuser || !comm_info->rungroup || !comm_info->command 
                      || !comm_info->psname) *rc = ERR_OPT_CONFIG;
                    else if( !*comm_info->runuser || !*comm_info->rungroup || !*comm_info->command
                      || !*comm_info->psname) *rc = ERR_OPT_CONFIG;
		}
	    }
	}
    }

    return comm_info;
}

/* --- */

int main( int narg, char **opts)

{
    int rc = RC_NORMAL, sysrc, off, log_level = DEF_LOG_LEVEL, log_facility = DEF_LOG_FACILITY;
    char *alias = 0, *errmsg = 0, *this_user = 0, *st = 0;
    struct comm_alias *rule = 0;
    struct syslog_opts *sopt = 0;
    xmlNodePtr root;

    /* --- */

    if( narg < 2) printf( "%s\n", ACTOR_HELP_MSG);
    else
    {
        /* Since we might SYSLOG about not being able to read the XML config,
         * which is where the SYSLOG config options are specified, we need to
         * call openlog() with the default parameters first.  We'll clean it
         * up later if a different "facility" is found in the XML.
         */
        openlog( ACTOR_SYSLOG_PREFIX, ACTOR_SYSLOG_FLAGS, log_facility);

        alias = opts[ 1];

        if( rc == RC_NORMAL)
        {
            root = source_config_file( &rc, ACTOR_CONFIG);
            if( rc == ERR_OPT_CONFIG) errmsg = ERRMSG_PARSE_CONFIG;
            else if( rc == ERR_BAD_FORMAT) errmsg = ERRMSG_XML_PARSE;
	}

        if( rc == RC_NORMAL)
        {
            sopt = get_syslog_options( root->children, &rc);
            if( rc == RC_NORMAL)
            {
                log_level = sopt->log_level;
                log_facility = sopt->log_facility;
	    }
            else if( rc == ERR_USE_DEFAULT) rc = RC_NORMAL;
            else if( rc == ERR_INVALID_DATA)
            {
                /* The log level, log facility or both were set to invalid values */
                errmsg = ERRMSG_BAD_CONFIG;
	    }
            else if( rc != RC_NORMAL)
            {
	        /* The only other possible options are unrecoverable, like ERR_MALLOC_FAILED */
                errmsg = ERRMSG_UNRECOVERABLE;
            }
	}

        if( rc == RC_NORMAL && log_level != LOG_NONE)
        {
            if( log_facility != DEF_LOG_FACILITY)
            {
                closelog();
                openlog( ACTOR_SYSLOG_PREFIX, ACTOR_SYSLOG_FLAGS, log_facility);
	    }

            if( LOG_DEBUG <= log_level) syslog( LOG_DEBUG, "Syslog level'%d' (%s) facility'%d' (%s)",
              log_level, PRSP( sopt->val_level), log_facility, PRSP( sopt->val_facility));
	}

        if( rc == RC_NORMAL)
        {
            for( off = 1; off < narg && rc == RC_NORMAL; off++)
              for( st = opts[ off]; *st && rc == RC_NORMAL; st++)
                if( !isprint( *st))
                {
                    rc = ERR_INVALID_DATA;
                    if( LOG_NOTICE <= log_level) syslog( LOG_NOTICE, "Non-printable args, UID'%d', alias'%s'",
                      getuid(), opts[ 1]);
                    errmsg = ERRMSG_INVALID_CHARS;
                }
	}

        if( rc == RC_NORMAL)
        {
            this_user = get_username( &rc, getuid());
            if( rc != RC_NORMAL) errmsg = ERRMSG_GET_USER;
            else if( LOG_DEBUG <= log_level) syslog( LOG_DEBUG, "Get calling user, rc=%d, name'%s'", rc, PRSP( this_user));
	}

        if( rc == RC_NORMAL)
        {
            rule = get_alias_rule( &rc, root, this_user, alias);

            if( rc == ERR_NO_MATCH) errmsg = ERRMSG_NO_SUCH_COMM;
            else if( rc == ERR_DISALLOW) errmsg = ERRMSG_NOT_ALLOWED;
            else if( rc != RC_NORMAL) errmsg = ERRMSG_UNRECOVERABLE;
            else
            {
                if( LOG_INFO <= log_level) syslog( LOG_INFO, "Rule, alias'%s' comm'%s' ps'%s' user'%s' group'%s'",
                  alias, PRSP( rule->command), PRSP( rule->psname), PRSP( rule->runuser), PRSP( rule->rungroup));
            }
	}

        if( rc == RC_NORMAL)
        {
            rc = switch_run_group( rule->rungroup);
            if( rc != RC_NORMAL) errmsg = ERRMSG_SET_GROUP;
            else if( LOG_DEBUG <= log_level) syslog( LOG_DEBUG, "Switched to group '%s'", rule->rungroup);
	}
       
        if( rc == RC_NORMAL)
        {
            rc = switch_run_user( rule->runuser);
            if( rc != RC_NORMAL) errmsg = ERRMSG_SET_USER;
            else if( LOG_DEBUG <= log_level) syslog( LOG_DEBUG, "Switched to user '%s'", rule->runuser);
	}

        if( rc == RC_NORMAL)
        {
            sysrc = setenv( ENV_IFS_VAR, ENV_IFS_VAL, DO_OVERWRITE);
            if( sysrc)
            {
                rc = ERR_SYS_CALL;
                errmsg = ERRMSG_UNRECOVERABLE;
	    }
	}

        if( rc == RC_NORMAL)
        {
            /* If we're supposed to use a "ps name" that's different from the program
             * to be run, then we need to swap in the "ps name".  Otherwise use the
             * command name specified in the config file for "ps".
             */
            if( strcmp( rule->psname, rule->command)) opts[ 1] = rule->psname;
            else opts[ 1] = rule->command;

            (void) execv( rule->command, opts + 1);

            rc = ERR_SYS_CALL;
            if( errno == EACCES || errno == ENOENT || errno == ENOEXEC || errno == ENOTDIR
              || errno == EPERM) errmsg = ERRMSG_BAD_COMMAND;
            else errmsg = ERRMSG_EXEC_FAIL;

            if( LOG_ERR <= log_level) syslog( LOG_ERR, "Exec failed, errno=%d, user'%s' group'%s' comm'%s'",
              errno, rule->runuser, rule->rungroup, rule->command);
	}
    }

    /* --- */

    if( errmsg) fprintf( stderr, "Err: rc=%d, %s\n", rc, errmsg);
    else if( rc != RC_NORMAL) fprintf( stderr, "Err: rc=%d\n", rc);

    if( LOG_ERR <= log_level)
    {
        if( !errmsg) errmsg = "(empty)";
        else if( !*errmsg) errmsg = "(empty)";

        if( !this_user) this_user = "(unknown)";
        else if( !*this_user) this_user = "(unknown)";

        if( !alias) alias = "(empty)";
        else if( !*alias) alias = "(empty)";

        syslog( LOG_ERR, "Err: rc=%d, user=%s, alias=%s, %s", rc, this_user, alias, errmsg);
    }

    return rc;
}
