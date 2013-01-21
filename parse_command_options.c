#if defined( linux) || defined( sun) || defined( __CYGWIN__) || defined( __APPLE__)
#include <string.h>
#include <stdio.h>
#endif
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include "parse_opt.h"
#include "err_ref.h"

/*
 * Revision history
 * ----------------
 * 1/19/13 -jj
 * -- Add a new option type OP_TYPE_LAST for options that mean everything 
 * --   else on the command line should be aggregated together.
 * 12/15/12 -jj
 * -- The original code didn't require a "-" prefix on flags, but that's too
 * --   loose for this application.
 * -- Make clean-up code parse the given value for "OP_TYPE_FLAG" options
 * --   and set the "parsed" field.  The value stored is an "int" that's
 * --   set by the value of the "flags" field if the option was seen.  And
 * --   if not, the default string is converted from char to int.
 * -- Also, change the code to allow for "--" style options and "--no-" too.
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
   int match, ii, op, *int_val, nconv, efp_pr_len, efp_ne_len, is_flag, nwords, off;
   char *st = 0, *first = 0, *dup_opt = 0, *invalid = 0, *copy = 0;
   float *fl_val = 0;
   struct option_set *curr = 0;
   struct word_chain *invlist = 0, *inv = 0, *endlist = 0;
   struct word_list *dup_opts = 0;

   *rc = 0;
   efp_pr_len = strlen( EXT_FLAG_PREFIX);
   efp_ne_len = strlen( EXT_FLAG_NEGATE);

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
      is_flag = 1;
      match = 0;
      st = first = opts[ ii];

      if( !strncmp( st, EXT_FLAG_NEGATE, efp_ne_len)) st += efp_ne_len;
      else if( !strncmp( st, EXT_FLAG_PREFIX, efp_pr_len)) st += efp_pr_len;
      else if( *st == '-') st++;
      else if( *st == '+') st++;
      else is_flag = 0;
      /* fprintf( stderr, "++DBG: first(%c) opt(%s) nopt(%d)\n", *first, PSSP( st), nopt); */

      if( is_flag) for( match = 0, op = 0; op < nopt && !match; op++)
      {
         /* fprintf( stderr, "++DBG: ii=%d, op=%d, compare (%s) (%s)\n", ii, op, PSSP( st),
              PSSP( plist[ op].name)); */
         if( !strcmp( st, plist[ op].name))
         {
           /* fprintf( stderr, "++DBG: ii=%d, op=%d, matched.\n", ii, op); */
            match = 1;
            plist[ op].flags |= OP_FL_FOUND;
            if( *first == '+' || !strncmp(first, EXT_FLAG_NEGATE, efp_ne_len)) plist[ op].flags &= ~OP_FL_SET;
            else plist[ op].flags |= OP_FL_SET;

            if( plist[ op].type != OP_TYPE_FLAG)
              if( ++ii < narg) plist[ op].val = opts[ ii];
            plist[ op].opt_num = ii;
            if( plist[ op].type == OP_TYPE_LAST) ii = narg;
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

      if( !curr->val) curr->val = curr->def;

      if( curr->type == OP_TYPE_FLAG)
      {
         int_val = (int *) malloc( sizeof *int_val);
         if( !int_val) *rc = ERR_MALLOC_FAILED;
         else
         {
            if( curr->opt_num) *int_val = curr->flags & OP_FL_SET;
            else
            {
               *int_val = strtol( curr->val, &invalid, 10);
               if( *invalid) 
               {
                  curr->flags |= OP_FL_INVALID;
                  *int_val = 0;
               }
               else if( *int_val) curr->flags |= OP_FL_SET;
	    }
            curr->parsed = (void *) int_val;
	 }
      }

      else if( curr->type == OP_TYPE_INT)
      {
         int_val = (int *) malloc( sizeof *int_val);
         if( !int_val) *rc = ERR_MALLOC_FAILED;
         else
         {
            *int_val = strtol( curr->val, &invalid, 10);
            if( *invalid)
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

      else if( curr->type == OP_TYPE_LAST)
      {
         dup_opts = (struct word_list *) malloc( sizeof *dup_opts);
         if( !dup_opts) *rc = ERR_MALLOC_FAILED;
         else
         {
            if( curr->flags & OP_FL_FOUND)
            {
                ii = curr->opt_num;
                nwords = narg - ii;
	    }
            else nwords = 1;

            dup_opts->count = nwords;
            dup_opts->words = (char **) malloc( nwords * (sizeof *dup_opts->words));

            if( curr->flags & OP_FL_FOUND)
            {
               for( off = 0; ii < narg && !*rc; ii++, off++)
               {
                  copy = strdup( opts[ ii]);
                  if( copy) dup_opts->words[ off] = copy;
                  else *rc = ERR_MALLOC_FAILED;
               }
	    }
            else
            {
               copy = strdup( curr->def);
               if( copy) dup_opts->words[ 0] = copy;
               else *rc = ERR_MALLOC_FAILED;
	    }
	 }
         if( !*rc) curr->parsed = (void *) dup_opts;
      }
      
   }

   /* --- */

   return invlist;
}

/* --- */

struct option_set *get_matching_option( int flag_num, struct option_set *opset, int nflags)

{
    struct option_set *walk, *match = 0;
    int off;

    for( off= 0, walk = opset; off < nflags; off++, walk++)
    {
        if( walk->num == flag_num)
        {
            if( !match) match = walk;
            else if( walk->opt_num > match->opt_num) match = walk;
	}
    }

    return( match);
}

/* --- */

void print_parse_summary( struct word_chain *extras, struct option_set *opset, int nflags)

{
    int off, word_off, *int_p = 0;
    struct option_set *co = 0;
    struct word_chain *walk = 0;
    struct word_list *wlist = 0;

    if( extras)
    {
        fprintf( stderr, "Extras '%s", extras->opt);
        for( walk = extras; walk; walk = walk->next)
          fprintf( stderr, " %s", walk->opt);
        fprintf( stderr, "'\n");
    }
    else fprintf( stderr, "No extraneous options.\n");

    /* Print out all options settings, includes defaults for unspecified options */
    fprintf( stderr, "Seq Num Typ Fl Opt\n");

    for( off= 0; off < nflags; off++)
    {
        co = opset + off;
        fprintf( stderr, "%2d. %3d %3d %2x ", off + 1, co->num, co->type, co->flags);
        fprintf( stderr, "%3d ", co->opt_num);
        if( co->type == OP_TYPE_INT || co->type == OP_TYPE_FLAG)
        {
            int_p = (int *) co->parsed;
            fprintf( stderr, "%d ", *int_p);
        }
        else if( co->type == OP_TYPE_CHAR) fprintf( stderr, "(%s) ", (char *) co->parsed);
        else if( co->type == OP_TYPE_FLOAT) fprintf( stderr, "%f ", *((float *) co->parsed));
        else if( co->type == OP_TYPE_LAST)
        {
            wlist = (struct word_list *) co->parsed;
            fprintf( stderr, "[ %d ", wlist->count);
            for( word_off = 0; word_off < wlist->count; word_off++)
              fprintf( stderr, " '%s'", wlist->words[ word_off]);
            fprintf( stderr, " ] ");
	}
        else fprintf( stderr, "/?/ ");
        fprintf( stderr, "(%s) (%s) ", co->name, co->val);
        fprintf( stderr, "\n");
    }

    return;
}
