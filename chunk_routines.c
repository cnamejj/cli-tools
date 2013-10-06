#ifndef __APPLE__
#include <malloc.h>
#endif
#include <string.h>
#include "err_ref.h"
#include "cli-sub.h"

/* --- */

struct data_chunk *allocate_one_chunk( struct data_chunk *curr, int buffer_size)

{
    struct data_chunk *new_chunk = 0;
    char *st = 0;

    new_chunk = (struct data_chunk *) malloc( (sizeof *new_chunk));
    if( new_chunk)
    {
        st = (char *) malloc( buffer_size);
        if( !st)
        {
            free( new_chunk);
            new_chunk = 0;
	}
        else
        {
            if( !curr) new_chunk->next = 0;
            else
            {
                new_chunk->next = curr->next;
                curr->next = new_chunk;
	    }
            new_chunk->size = 0;
            new_chunk->data = st;
            memset( new_chunk->data, '\0', buffer_size);
	}
    }

    return( new_chunk);
}

/* --- */

void free_chunk_chain( struct data_chunk *chain)

{
    struct data_chunk *curr = 0, *walk = 0;

    for( curr = chain; curr; )
    {
        walk = curr->next;
        if( curr->data) free( curr->data);
        free( curr);
        curr = walk;
    }

    return;
}
