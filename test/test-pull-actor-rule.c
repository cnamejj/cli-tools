#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "../err_ref.h"
#include "../cli-sub.h"
#include "test-pull-actor-rule.h"

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
