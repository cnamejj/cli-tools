#ifndef ___CNAMEjj_ACTOR_H__

#define ___CNAMEjj_ACTOR_H__

/* --- */

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <syslog.h>

/* --- */

#define BLANK ' '
#define EOS '\0';

#define ACTOR_CONFIG "/usr/local/etc/actor.xml"

#define ACTOR_HELP_MSG "Syntax is: actor command-alias\n"

#define ACTOR_SYSLOG_PREFIX "Actor:"
#define ACTOR_SYSLOG_FLAGS LOG_PID

#define ERRMSG_PARSE_CONFIG "Unable to read config file."
#define ERRMSG_XML_PARSE "Malformed XML config file."
#define ERRMSG_GET_USER "Can't map UID to username."
#define ERRMSG_NO_SUCH_COMM "Unrecognized command requested."
#define ERRMSG_NOT_ALLOWED "Command not executable by this user."
#define ERRMSG_UNRECOVERABLE "Unrecoverable system error."
#define ERRMSG_SET_GROUP "Can't change GID to the necessary group."
#define ERRMSG_SET_USER "Can't change UID to the necessary userid."
#define ERRMSG_BAD_COMMAND "Command/Path requested invalid, missing, or permission denied."
#define ERRMSG_EXEC_FAIL "Attempt to execute the command caused an unrecoverable error."
#define ERRMSG_BAD_CONFIG "The configuration file contains invalid settings."
#define ERRMSG_INVALID_CHARS "Unprintable characters in commandline."

#define ENV_IFS_VAR "IFS"
#define ENV_IFS_VAL " \t\n"

#define DO_OVERWRITE 1

#define LOG_NONE -1

#define DEF_LOG_LEVEL LOG_ERR
#define DEF_LOG_FACILITY LOG_USER

#define ALIAS_TAG "command"
#define ALLOW_TAG "allow"

#define IS_ALIAS "alias"
#define IS_USER "user"
#define IS_RUN_USER "runuser"
#define IS_RUN_GROUP "rungroup"
#define IS_COMMAND "exec"
#define IS_PS_NAME "psname"

#define SYSLOG_TAG "syslog"

#define IS_LOG_LEVEL "level"
#define IS_LOG_FACILITY "facility"

#define IS_LOG_LEVEL_NONE "none"
#define IS_LOG_LEVEL_DEBUG "debug"
#define IS_LOG_LEVEL_INFO "info"
#define IS_LOG_LEVEL_NOTICE "notice"
#define IS_LOG_LEVEL_WARNING "warning"
#define IS_LOG_LEVEL_ERROR "error"
#define IS_LOG_LEVEL_ERR "err"

#define IS_LOG_FACIL_AUTH "auth"
#define IS_LOG_FACIL_AUTHPRIV "authpriv"
#define IS_LOG_FACIL_LOCAL0 "local0"
#define IS_LOG_FACIL_LOCAL1 "local1"
#define IS_LOG_FACIL_LOCAL2 "local2"
#define IS_LOG_FACIL_LOCAL3 "local3"
#define IS_LOG_FACIL_LOCAL4 "local4"
#define IS_LOG_FACIL_LOCAL5 "local5"
#define IS_LOG_FACIL_LOCAL6 "local6"
#define IS_LOG_FACIL_LOCAL7 "local7"
#define IS_LOG_FACIL_USER "user"

/* --- */

#define PRSP( STRING_POINTER) STRING_POINTER ? STRING_POINTER : ""

/* --- */

struct syslog_opts {
    int log_level, log_facility;
    char *val_level, *val_facility;
};

struct comm_alias
{
   char *runuser, *rungroup, *psname, *command;
};

/* --- */

struct comm_alias *get_alias_rule( int *rc, xmlNodePtr node, char *user, char *alias);

struct syslog_opts *get_syslog_options( xmlNodePtr head, int *rc);

#endif
