#include <string.h>

#include "cli-sub.h"

/* --- */

void free_value_chain( struct value_chain *vchain)

{
    struct value_chain *curr = 0, *walk;

    for( walk = vchain; walk; )
    {
        curr = walk;
        walk = walk->next;

        curr->next = 0;
        curr->val = 0;
        curr->parsed = 0;
        free( curr);
    }

    return;
}
