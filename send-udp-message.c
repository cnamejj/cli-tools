#include <stdlib.h>
#include <stdio.h>
#include "parse_opt.h"
#include "send-udp-message.h"

/* --- */

int main( int narg, char **opts)

{
   int rc = 0, off, *int_p = 0;
   struct option_set opset[] = {
     { OP_DEST_BOTH, OP_TYPE_CHAR, OP_FL_BLANK, FL_DEST_BOTH, 0, DEF_DEST_BOTH, 0, 0 },
     { OP_DEST_BOTH, OP_TYPE_CHAR, OP_FL_BLANK, FL_DEST_BOTH_2, 0, DEF_DEST_BOTH, 0, 0 },
     { OP_DEST_BOTH, OP_TYPE_CHAR, OP_FL_BLANK, FL_DEST_BOTH_3, 0, DEF_DEST_BOTH, 0, 0 },
     { OP_MESSAGE, OP_TYPE_CHAR, OP_FL_BLANK, FL_MESSAGE, 0, DEF_MESSAGE, 0, 0 },
     { OP_MESSAGE, OP_TYPE_CHAR, OP_FL_BLANK, FL_MESSAGE_2, 0, DEF_MESSAGE, 0, 0 },
     { OP_MESSAGE, OP_TYPE_CHAR, OP_FL_BLANK, FL_MESSAGE_3, 0, DEF_MESSAGE, 0, 0 },
     { OP_PORT, OP_TYPE_INT, OP_FL_BLANK, FL_PORT, 0, DEF_PORT, 0, 0 },
     { OP_PORT, OP_TYPE_INT, OP_FL_BLANK, FL_PORT_2, 0, DEF_PORT, 0, 0 },
     { OP_HOST, OP_TYPE_CHAR, OP_FL_BLANK, FL_HOST, 0, DEF_HOST, 0, 0 },
     { OP_HOST, OP_TYPE_CHAR, OP_FL_BLANK, FL_HOST_2, 0, DEF_HOST, 0, 0 },
     { OP_IPV4, OP_TYPE_INT, OP_FL_BLANK, FL_IPV4, 0, DEF_IPV4, 0, 0 },
     { OP_IPV4, OP_TYPE_INT, OP_FL_BLANK, FL_IPV4_2, 0, DEF_IPV4, 0, 0 },
     { OP_IPV6, OP_TYPE_INT, OP_FL_BLANK, FL_IPV6, 0, DEF_IPV6, 0, 0 },
     { OP_IPV6, OP_TYPE_INT, OP_FL_BLANK, FL_IPV6_2, 0, DEF_IPV6, 0, 0 },
   };
   struct option_set *co = 0;
   struct word_chain *plain_opts = 0, *walk = 0;
   int nflags = (sizeof opset) / (sizeof opset[0]);
   char *sep = 0;
   
   plain_opts = parse_command_options( &rc, opset, nflags, narg, opts);

   printf( "rc=%d extra(", rc);
   sep = "";
   for( walk = plain_opts; walk; )
   {
      printf( "%s%s", sep, walk->opt);
      walk = walk->next;
      if( walk) sep = " ";
      else sep = "";
   }
   printf( ")\n");

   for( off= 0; off < nflags; off++)
   {
      co = opset + off;
      printf( "%d. %d %d %2x (%s) (%s) ", off, co->num, co->type, co->flags, co->name, co->val);
      if( co->type == OP_TYPE_INT)
      {
         int_p = (int *) co->parsed;
         printf( "%d", (int) *int_p);
      }
      else printf( "(%s)", (char *) co->parsed);
      printf( " %d\n", co->opt_num);
   }

   exit( rc );
}
