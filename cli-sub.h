#ifndef ___CNAMEjj_CLI_SUB_H__

#define ___CNAMEjj_CLI_SUB_H__

/*
 * Revision history
 * ----------------
 * 1/21/13 -jj
 * -- Combine with other header files for "libCLI" routines, and rename...
 * 12/11/12
 * -- Copied from "junction-ac" code and tweaked to add in error value constants
 *
 * Todo
 * ----
 */
  
#include <sys/stat.h>

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

/* --- */

struct word_chain *parse_command_options( int *rc, struct option_set *plist, int nopt, int narg, char **opts);

struct option_set *get_matching_option( int flag_num, struct option_set *opset, int nflags);

void print_parse_summary( struct word_chain *, struct option_set *, int);

mode_t convert_to_mode( int);

char *gsub_string( int *rc, char *template, struct sub_list *patts);

/* --- */

#endif
