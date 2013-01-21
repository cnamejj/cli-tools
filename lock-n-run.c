#include <stdlib.h>
#include <stdio.h>

#include "parse_opt.h"
#include "err_ref.h"

/* --- */

#define USE_DEFAULT "-"
#define NO_SWITCH_NEEDED USE_DEFAULT
#define SUB_UID "%{uid}"
#define PER_UID_LOCKFILE "/var/tmp/%{uid}.__lockfile__"

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
| <-user run-user> \
| <-group group-user> \
| -comm command...\n\
"

/* --- */

int main( int narg, char **opts)

{
    int rc = RC_NORMAL;
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
    struct word_chain *extra_opts = 0;
    int nflags = (sizeof opset) / (sizeof opset[0]);

    extra_opts = parse_command_options( &rc, opset, nflags, narg, opts);

    print_parse_summary( extra_opts, opset, nflags);

    exit( rc);
}
