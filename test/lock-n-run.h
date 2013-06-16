#ifndef ___CNAMEjj_LOCK_N_RUN_H__

#define ___CNAMEjj_LOCK_N_RUN_H__

/* --- */

#define DEBUG_NONE 0
#define DEBUG_LOW1 1
#define DEBUG_LOW2 2
#define DEBUG_LOW DEBUG_LOW2
#define DEBUG_LOW3 3
#define DEBUG_MEDIUM1 4
#define DEBUG_MEDIUM2 5
#define DEBUG_MEDIUM DEBUG_MEDIUM2
#define DEBUG_MEDIUM3 6
#define DEBUG_HIGH1 7
#define DEBUG_HIGH2 8
#define DEBUG_HIGH DEBUG_HIGH2
#define DEBUG_HGIH3 9
#define DEBUG_NOISY1 10
#define DEBUG_NOISY2 11
#define DEBUG_NOISY DEBUG_NOISY2
#define DEBUG_NOISY3 12

/* --- */

#define USE_DEFAULT "-"
#define NO_SWITCH_NEEDED USE_DEFAULT
#define SUB_UNAME "%{uname}"
#define DEFAULT_LOCKFILE "/var/tmp/%{psname}.__lockfile__"
#define TIME_FORMAT "%y-%m-%d"

#define ENV_IFS_VAR "IFS"
#define ENV_IFS_VAL " \t\n"
#define DO_OVERWRITE 1

#define OP_LOCKFILE 1
#define OP_PSNAME 2
#define OP_LOCKMODE 3
#define OP_WAIT 4
#define OP_COMM 5
#define OP_DEBUG 6
#define OP_USER 7
#define OP_GROUP 8
#define OP_HELP 9

#define FL_LOCKFILE "lockfile"
#define FL_LOCKFILE_2 "l"
#define FL_LOCKFILE_3 "file"
#define FL_LOCKFILE_4 "f"
#define FL_PSNAME "psname"
#define FL_PSNAME_2 "p"
#define FL_LOCKMODE "mode"
#define FL_LOCKMODE_2 "m"
#define FL_WAIT "wait"
#define FL_WAIT_2 "w"
#define FL_COMM "comm"
#define FL_COMM_2 "c"
#define FL_DEBUG "debug"
#define FL_USER "user"
#define FL_USER_2 "u"
#define FL_GROUP "group"
#define FL_GROUP_2 "g"
#define FL_HELP "help"

#define DEF_LOCKFILE USE_DEFAULT
#define DEF_PSNAME USE_DEFAULT
#define DEF_LOCKMODE "644"
#define DEF_WAIT "0"
#define DEF_COMM "/bin/date"
#define DEF_DEBUG "0"
#define DEF_USER NO_SWITCH_NEEDED
#define DEF_GROUP NO_SWITCH_NEEDED
#define DEF_HELP "0"
#define DEF_ERRMSG "There will be a better error message here soon"

#define ERR_NULL_PSNAME "The process name to be shown in 'ps' was an empty string"
#define ERR_NULL_RUNUSER "The run user cannot be an empty string"
#define ERR_NULL_RUNGROUP "The run group cannot be an empty string"
#define ERR_NULL_LOCKFILE "Lockfile requested was an empty string"
#define ERR_NULL_COMMAND "The command to run can't be an empty string"
#define ERR_INVALID_RUN_USER "Invalid characters found in requested run user"
#define ERR_INVALID_RUN_GROUP "Invalid characters found in requested run group"
#define ERR_INVALID_LOCK_MODE "File permissions requested for lockfile are invalid"
#define ERR_SETX_LOCK_MODE "Lockfile file permissions can't include set UID, set GID or sticky bits"
#define ERR_MAX_PATH_LEN "Filename too long"
#define ERR_INVALID_FILENAME "Invalid character(s) in filename"
#define ERR_INVALID_PSNAME "Invalid characters found in visible 'ps' name"
#define ERR_SETUID_FAIL "Unable to switch process UID to the requested user"
#define ERR_SETGID_FAIL "Unable to switch process GID to the requested group"
#define ERR_LOCK_OPEN_FAIL "Unable to open the requested lockfile"
#define ERR_LOCK_UNAVAILABLE "Another process has lock already"
#define ERR_LOCK_TIMEOUT "Timed out waiting for lock"
#define ERR_LOCK_FAIL "System call to establish lock failed with an unrecoverable error"
#define ERR_LOCK_WRITE_FAIL "Unable to record process information in lockfile"
#define ERR_EXEC_BAD_COMMAND "Command/Path requested invalid, missing, or permission denied"
#define ERR_EXEC_FAIL "Attempt to execute the command caused an unrecoverable error"
#define ERR_GETUSERNAME_FAIL "Unable to get the username associated with the current UID"
#define ERR_GETGROUPNAME_FAIL "Unable to get the group name associated with the current GID"
#define ERR_SETENV_FAIL "Unable to setup environment before running the requested command"

/* --- */

#define MSG_SHOW_SYNTAX "\n\
Syntax is: %s <options> --comm command...\n\
Options are:\n\
  <--lockfile name> | <--file name> | <-l name> | <-f name>\n\
  <--mode mode> | <-m mode> \n\
  <--user name> | <-u name>\n\
  <--group name> | <-g name>\n\
  <--wait secs> | <-w secs> \n\
  <--psname name> | <-p name>\n\
  <--help>\n\
  <--debug level>\n\
"

/* --- */

#endif
