#ifndef ___CNAMEjj_CLI_SUB_H__

#define ___CNAMEjj_CLI_SUB_H__

/*
 * Revision history
 * ----------------
 * 6/10/13 -jj
 # -- Include bits needed for "xml2-util" routines.
 * 1/21/13 -jj
 * -- Combine with other header files for "libCLI" routines, and rename...
 * 12/11/12
 * -- Copied from "junction-ac" code and tweaked to add in error value constants
 *
 * Todo
 * ----
 */
  
#include <sys/stat.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

/* --- */

#define SUBSET_READ  0x4
#define SUBSET_WRITE 0x2
#define SUBSET_EXEC  0x1

#define OP_TYPE_CHAR  1
#define OP_TYPE_FLAG  2
#define OP_TYPE_INT   3
#define OP_TYPE_VOID  4
#define OP_TYPE_FLOAT 5
#define OP_TYPE_LAST  6

#define OP_FL_BLANK   0x00
#define OP_FL_FOUND   0x01
#define OP_FL_SET     0x02
#define OP_FL_INVALID 0x04

#define EXT_FLAG_PREFIX "--"
#define EXT_FLAG_NEGATE "--no-"

#define INT_DISPLAY_LEN 11

#define HEX_BASE 16

#define ALIAS_TAG "command"
#define ALLOW_TAG "allow"

#define IS_ALIAS "alias"
#define IS_USER "user"
#define IS_RUN_USER "runuser"
#define IS_RUN_GROUP "rungroup"
#define IS_COMMAND "exec"
#define IS_PS_NAME "psname"

#define BLANK ' '
#define EOS '\0';

#define GNM_FIRST 0
#define GNM_NEXT 1
#define GNM_LAST 2

/* --- */

struct option_set {
   unsigned int num, type, flags;
   char *name, *val, *def;
   void *parsed;
   int opt_num;
};

struct word_chain {
   char *opt;
   struct word_chain *next;
};

struct word_list {
   int count;
   char **words;
};

struct sub_list {
   char *from, *to;
   struct sub_list *next;
};

struct comm_alias
{
   char *runuser, *rungroup, *psname, *command;
};

/* --- */

struct word_chain *parse_command_options( int *rc, struct option_set *plist, int nopt, int narg, char **opts);

struct option_set *get_matching_option( int flag_num, struct option_set *opset, int nflags);

void print_parse_summary( struct word_chain *, struct option_set *, int);

mode_t convert_to_mode( int);

char *gsub_string( int *rc, char *template, struct sub_list *patts);

char *get_groupname( int *rc, uid_t gid);

char *get_username( int *rc, uid_t uid);

int switch_run_user( char *user);

int switch_run_group( char *group);

char *int_to_str( char *buff, int buff_len, int source, char *format);

char *build_syscall_errmsg( char *syscall, int sysrc);

char *hexdigits_to_string( int *rc, int *msglen, char *hex);

xmlNodePtr search_node_list( xmlNodePtr head, char *node_name, char *attrib_name, char *attrib_val);

char *dup_attrib_value( int *rc, xmlNodePtr curr, char *attrib_name);

char *get_attrib_value( int *rc, xmlNodePtr curr, char *attrib_name);

xmlAttr *search_attrib_list( xmlAttr *head, char *attrib_name, char *attrib_val);

xmlNodePtr get_node_match( xmlNodePtr head, char *node_name, int match_type);

xmlNodePtr source_config_file( int *rc, char *config);

/* --- */

#endif


