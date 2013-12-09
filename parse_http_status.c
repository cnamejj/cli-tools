#include <string.h>
#include "cli-sub.h"

struct http_status_response *parse_http_status( char *line)

{
    int code, err = 0;
    char *st = 0, *delim = 0, *pos = 0;
    struct http_status_response *resp = 0;

    resp = (struct http_status_response *) malloc( sizeof *resp);
    if( resp)
    {
        resp->code = 0;
        resp->version = 0;
        resp->reason = 0;

        delim = index( line, BLANK_CH);
        st = dup_memory( line, delim - 1);
        if( st)
        {
            resp->version = st;

            pos = delim + 1;
            delim = index( pos, BLANK_CH);
            st = dup_memory( pos, delim - 1);
            if( st)
            {
                code = strtol( st, &pos, 10);
                if( !pos) err = 1;
                else if( *pos) err = 1;
                else
                {
                    resp->code = code;
                    pos = strdup( delim + 1);
                    if( pos) resp->reason = pos;
                }

                free( st);
            }
        }

        if( !resp->version || !resp->reason || err)
        {
            if( resp->version) free( resp->version);
            if( resp->reason) free( resp->reason);
            free( resp);
            resp = 0;
        }
    }

    return( resp);
}
