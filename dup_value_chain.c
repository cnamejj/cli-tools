#include <string.h>

#include "cli-sub.h"
#include "err_ref.h"

/* --- */

struct value_chain *dup_value_chain( int *rc, struct value_chain *vchain)

{
    struct value_chain *copy = 0, *curr = 0, *walk, *orig;

    for( orig = vchain; *rc == RC_NORMAL && orig; orig = orig->next)
    {
        walk = (struct value_chain *) malloc( sizeof *walk);
        if( !walk) *rc = ERR_MALLOC_FAILED;
        else
        {
            if( !copy) copy = walk;
            walk->opt_num = orig->opt_num;
            walk->flags = orig->flags;
            /* We don't want to duplicate the data ref'd by "val" and "parsed", just copy the pointers */
            walk->val = orig->val;
            walk->parsed = orig->parsed;
            walk->next = 0;
            if( curr) curr->next = walk;
            curr = walk;
	}
    }

    /* --- */

    if( *rc != RC_NORMAL)
    {
        for( walk = copy; walk; )
        {
            curr = walk;
            walk = curr->next;
            curr->val = 0;
            curr->parsed = 0;
            curr->next = 0;
            free( curr);
	}

        copy = 0;
    }

    /* --- */

    return( copy);
}
