#include <string.h>
#include "cli-sub.h"

/* --- */

struct word_chain *add_to_word_chain( struct word_chain *anchor, char *data)

{
    struct word_chain *entry = 0;

    if( data) if( *data)
    {
        entry = (struct word_chain *) malloc( sizeof *entry);

        if( entry)
        {
            entry->next = 0;
            entry->opt = strdup( data);
            if( !entry->opt)
            {
                free( entry);
                entry = 0;
	    }
        }
    }

    if( anchor) anchor->next = entry;

    return( entry);
}
