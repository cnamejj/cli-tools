#include <stdlib.h>
#include <stdio.h>
#include "parse_opt.h"
#include "send-udp-message.h"

#define DEBUG_OPTION_PARSING

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
    struct word_chain *extra_opts = 0, *walk = 0;
    int nflags = (sizeof opset) / (sizeof opset[0]);
    char *sep = 0;

    /* --- */
   
    extra_opts = parse_command_options( &rc, opset, nflags, narg, opts);

#ifdef DEBUG_OPTION_PARSING
    /* --- */

    printf( "rc=%d extra(", rc);
    sep = "";
    for( walk = extra_opts; walk; )
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
        printf( "%2d. %d %d %2x (%s) (%s) ", off, co->num, co->type, co->flags, co->name, co->val);
        if( co->type == OP_TYPE_INT)
        {
            int_p = (int *) co->parsed;
            printf( "%d", (int) *int_p);
        }
        else if( co->type == OP_TYPE_CHAR) printf( "(%s)", (char *) co->parsed);
        else printf( "/?/");
        printf( " %d\n", co->opt_num);
    }
#endif

    /* --- */

    co = get_matching_option( OP_DEST_BOTH, opset, nflags);
/*    if( !co) ...should never happen, bail...
    ...call routine to parse the "destination" option, needs to split the pieces...
 */

    co = get_matching_option( OP_MESSAGE, opset, nflags);

    co = get_matching_option( OP_PORT, opset, nflags);

    co = get_matching_option( OP_HOST, opset, nflags);

    co = get_matching_option( OP_IPV4, opset, nflags);

    co = get_matching_option( OP_IPV6, opset, nflags);
    

    /* ... */   

    exit( rc );
}
