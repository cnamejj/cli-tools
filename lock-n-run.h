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

#define OP_LOCKFILE 1
#define OP_PSNAME 2
#define OP_LOCKMODE 3
#define OP_WAIT 4
#define OP_COMM 5
#define OP_DEBUG 6
#define OP_USER 7
#define OP_GROUP 8

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

#define DEF_LOCKFILE USE_DEFAULT
#define DEF_PSNAME USE_DEFAULT
#define DEF_LOCKMODE "644"
#define DEF_WAIT "0"
#define DEF_COMM "/bin/date"
#define DEF_DEBUG "0"
#define DEF_USER NO_SWITCH_NEEDED
#define DEF_GROUP NO_SWITCH_NEEDED

/* --- */

#define MSG_SHOW_SYNTAX "\
Syntax is: %s \
| <-lockfile name> \
| <-psname name> \
| <-mode mode> \
| <-wait secs> \
| <-user name> \
| <-group name> \
| -comm command...\n\
"

/* --- */

#endif
