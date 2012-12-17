#ifndef ___jj_PARSE_COMM_OPT_H__

#define ___jj_PARSE_COMM_OPT_H__

/*
 * Revision history
 * ----------------
 * 12/11/12
 * -- Copied from "junction-ac" code and tweaked to add in error value constants
 *
 * Todo
 * ----
 */
  
/* --- */

#define OP_TYPE_CHAR  1
#define OP_TYPE_FLAG  2
#define OP_TYPE_INT   3
#define OP_TYPE_VOID  4
#define OP_TYPE_FLOAT 5

#define OP_FL_BLANK   0x00
#define OP_FL_FOUND   0x01
#define OP_FL_SET     0x02
#define OP_FL_INVALID 0x04

#define EXT_FLAG_PREFIX "--"
#define EXT_FLAG_NEGATE "--no-"

/* --- */

#define RC_NORMAL 0

#define ERR_SYS_CALL 106
#define ERR_MALLOC_FAILED 115
#define ERR_GETHOST_FAILED 135
#define ERR_INVALID_DATA 141
#define ERR_OPT_CONFIG 159

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

/* --- */

struct word_chain *parse_command_options( int *rc, struct option_set *plist, int nopt, int narg, char **opts);

/* --- */

#endif
