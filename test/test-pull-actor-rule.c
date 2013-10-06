#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "../err_ref.h"
#include "../cli-sub.h"
#include "../actor.h"
#include "test-pull-actor-rule.h"


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
    int rc = RC_NORMAL;
    char *conf = 0, *user = 0, *comm = 0, *log_level = 0, *log_facility = 0;
    xmlDocPtr doc;
    xmlNodePtr root, syslog;
    struct comm_alias *rule = 0;


    if( narg < 4) exit( 1);

    conf = opts[ 1];
    user = opts[ 2];
    comm = opts[ 3];

    doc = xmlParseFile( conf);
    if( !doc)
    {
        fprintf( stderr, "Parse error.\n");
        exit( 1);
    }

    root = xmlDocGetRootElement( doc);
    if( !root)
    {
        fprintf( stderr, "No document root.\n");
        xmlFreeDoc( doc);
        exit( 1);
    }

    printf( "Document root, name '%s'\n", root->name);

    syslog = get_node_match( root->children, SYSLOG_TAG, GNM_FIRST);
    if( syslog)
    {
        log_level = dup_attrib_value( &rc, syslog, IS_LOG_LEVEL);
        if( rc == RC_NORMAL) log_facility = dup_attrib_value( &rc, syslog, IS_LOG_FACILITY);
        if( rc == RC_NORMAL) printf( "Syslog(F): level'%s' facility'%s'\n", log_level, log_facility);
        else printf( "Err: rc=%d trying to get syslog settings.\n", rc);
    }

    syslog = get_node_match( syslog, SYSLOG_TAG, GNM_NEXT);
    if( syslog)
    {
        log_level = dup_attrib_value( &rc, syslog, IS_LOG_LEVEL);
        if( rc == RC_NORMAL) log_facility = dup_attrib_value( &rc, syslog, IS_LOG_FACILITY);
        if( rc == RC_NORMAL) printf( "Syslog(N): level'%s' facility'%s'\n", log_level, log_facility);
        else printf( "Err: rc=%d trying to get syslog settings.\n", rc);
    }

    syslog = get_node_match( root->children, SYSLOG_TAG, GNM_LAST);
    if( syslog)
    {
        log_level = dup_attrib_value( &rc, syslog, IS_LOG_LEVEL);
        if( rc == RC_NORMAL) log_facility = dup_attrib_value( &rc, syslog, IS_LOG_FACILITY);
        if( rc == RC_NORMAL) printf( "Syslog(L): level'%s' facility'%s'\n", log_level, log_facility);
        else printf( "Err: rc=%d trying to get syslog settings.\n", rc);
    }

    syslog = get_node_match( syslog, SYSLOG_TAG, GNM_NEXT);
    if( syslog)
    {
        log_level = dup_attrib_value( &rc, syslog, IS_LOG_LEVEL);
        if( rc == RC_NORMAL) log_facility = dup_attrib_value( &rc, syslog, IS_LOG_FACILITY);
        if( rc == RC_NORMAL) printf( "Syslog(X): level'%s' facility'%s'\n", log_level, log_facility);
        else printf( "Err: rc=%d trying to get syslog settings.\n", rc);
    }

    rule = get_alias_rule( &rc, root, user, comm);

    printf( "Search: alias'%s' user'%s' conf'%s'\n", comm, user, conf);

    if( rc == RC_NORMAL && rule) printf( "User:: '%s'\nGroup: '%s'\nPSNa:: '%s'\nComm:: '%s'\n", rule->runuser, rule->rungroup, rule->psname, rule->command);
    else printf( "Error, rc=%d, %s\n", rc, cli_strerror( rc));

    exit( 0);
}
