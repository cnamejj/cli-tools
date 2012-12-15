#if defined( linux) || defined( sun) || defined( __CYGWIN__)
#include <string.h>
#include <stdio.h>
#endif
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>
#include "parse_opt.h"

/*
 * Revision history
 * ----------------
 * 12/11/12 -jj
 * -- Pulled from the "junction-ac" code with minor changes made.
 *
 * Todo
 * ----
 */
  

/* --- */

struct word_chain *parse_command_options( int *rc, struct option_set *plist,
  int nopt, int narg, char **opts)

{
   int match, ii, op, *int_val, nconv;
   char *st = 0, *first = 0, *dup_opt = 0;
   float *fl_val = 0;
   struct option_set *curr = 0;
   struct word_chain *invlist = 0, *inv = 0, *endlist = 0;

   *rc = 0;

   /* --- */

   for( op = 0; op < nopt && !*rc; op++)
   {
      curr = plist + op;
      curr->val = 0;
      curr->flags = OP_FL_BLANK;
      curr->parsed = 0;
      curr->opt_num = 0;
   }

   /* ---
    * Options, options, options...
    */

   for( ii = 1, *rc = 0; ii < narg && !*rc; ii++)
   {
      st = first = opts[ ii];
      if( *st == '-') st++;
      else if( *st == '+') st++;
      /* fprintf( stderr, "++DBG: first(%c) opt(%s) nopt(%d)\n", *first, PSSP( st), nopt); */

      for( match = 0, op = 0; op < nopt && !match; op++)
      {
         /* fprintf( stderr, "++DBG: ii=%d, op=%d, compare (%s) (%s)\n", ii, op, PSSP( st), PSSP( plist[ op].name)); */
         if( !strcmp( st, plist[ op].name))
         {
           /* fprintf( stderr, "++DBG: ii=%d, op=%d, matched.\n", ii, op); */
            match = 1;
            if( *first == '+') plist[ op].flags &= ~OP_FL_SET;
            else plist[ op].flags |= OP_FL_SET;

            if( plist[ op].type != OP_TYPE_FLAG)
              if( ++ii < narg) plist[ op].val = opts[ ii];
            plist[ op].opt_num = ii;
	 }
      }

      if( !match)
      {
         inv = (struct word_chain *) malloc( (sizeof *inv));
         dup_opt = strdup( opts[ ii]);
         if( !inv || !dup_opt) *rc = ERR_MALLOC_FAILED;
         else
         {
            if( !endlist) invlist = endlist = inv;
            else endlist->next = inv;
            inv->next = 0;
            inv->opt = dup_opt;
            endlist = inv;
         }
      }
   }

   for( op = 0; op < nopt && !*rc; op++)
   {
      curr = plist + op;

      if( !curr->val) if( curr->type != OP_TYPE_FLAG)
        curr->val = curr->def;

      if( curr->type == OP_TYPE_INT)
      {
         int_val = (int *) malloc( sizeof *int_val);
         if( !int_val) *rc = ERR_MALLOC_FAILED;
         else
         {
            errno = 0;
            *int_val = atoi( curr->val);
            if( errno)
            {
               curr->flags |= OP_FL_INVALID;
               *int_val = 0;
            }
            curr->parsed = (void *) int_val;
         }
      }

      else if( curr->type == OP_TYPE_FLOAT)
      {
         fl_val = (float *) malloc( sizeof *fl_val);
         if( !fl_val) *rc = ERR_MALLOC_FAILED;
         else
         {
            nconv = sscanf( curr->val, "%f", fl_val);
            if( nconv != 1)
            {
               curr->flags |= OP_FL_INVALID;
               *fl_val = 0.0;
            }
            curr->parsed = (void *) fl_val;
         }
      }
      
      else if( curr->type == OP_TYPE_CHAR || curr->type == OP_TYPE_VOID)
      {
         curr->parsed = (void *) strdup( curr->val);
         if( !curr->parsed) *rc = ERR_MALLOC_FAILED;
      }
      
   }

   /* --- */

   return invlist;
}
