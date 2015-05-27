#include <string.h>
#include "err_ref.h"
#include "cli-sub.h"

char *get_cgi_data( int *rc)

{
    char *cgi_data = 0, *query_string = 0, *st = 0;
    int post_data_len = 0, chunksize, has_query_data;
    struct data_chunk *new_chunk = 0, *curr_chunk = 0, *first_chunk = 0, *walk = 0;

    /* --- */

    has_query_data = 0;
    query_string = getenv( ENV_QUERY_STRING);
    if( query_string) if( *query_string) has_query_data = 1;

    if( has_query_data)
    {
        cgi_data = strdup( query_string);
        if( !cgi_data) *rc = ERR_MALLOC_FAILED;
    }
    else
    {
        for( ; *rc == RC_NORMAL && !feof( stdin) && !ferror( stdin); )
        {
            new_chunk = allocate_one_chunk( curr_chunk, CHUNK_MAX);
            if( !new_chunk) *rc = ERR_MALLOC_FAILED;
            else
            {
                if( !first_chunk) first_chunk = new_chunk;

                chunksize = fread( new_chunk->data, BYTE_SIZE, CHUNK_MAX, stdin);

                if( chunksize)
                {
                    new_chunk->size = chunksize;
                    curr_chunk = new_chunk;
                    post_data_len += chunksize;
                }
                else
                {
                    if( curr_chunk) curr_chunk->next = 0;
                    free_chunk_chain( new_chunk);
                    if( first_chunk == new_chunk) first_chunk = 0;
                    new_chunk = 0;
		}
            }
	}

        if( *rc == RC_NORMAL && ferror( stdin)) *rc = ERR_READ_FAILED;

        if( *rc == RC_NORMAL)
        {
            cgi_data = (char *) malloc( post_data_len + 1);
            if( !cgi_data) *rc = ERR_MALLOC_FAILED;
            else
            {
                st = cgi_data;
                for( walk = first_chunk; walk; walk = walk->next)
                {
                    memcpy( st, walk->data, walk->size);
                    st += walk->size;
                    *st = '\0';
		}

                free_chunk_chain( first_chunk);
                first_chunk = 0;
	    }
	}
    }

    /* --- */

    return( cgi_data);
}
