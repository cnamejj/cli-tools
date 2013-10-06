#include <errno.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <stdlib.h>

#include "cli-sub.h"
#include "err_ref.h"

/* --- */

char *hexdigits_to_string( int *rc, int *result_len, char *hex)

{
    char *msg = 0, *st_hex = 0, *st_msg = 0, *co = 0,
          conv[ 3] = { '\0', '\0', '\0'};
    int hex_len, msg_len = 0, buff_size, off;


    *rc = RC_NORMAL;

    if( !hex) *rc = ERR_INVALID_DATA;
    else if( !*hex) *rc = ERR_INVALID_DATA;
    else
    {
        hex_len = strlen( hex);
        msg_len = hex_len / 2;

        buff_size = msg_len + 2;
        msg = (char *) malloc( buff_size);

        st_hex = hex;
        st_msg = msg;

        if( !msg) *rc = ERR_MALLOC_FAILED;
        else
        {
            co = conv;
            off = 0;

            if( hex_len % 2)
            {
                msg_len++;
                off = 1;

                conv[ 0] = *st_hex;
                errno = 0;
                *st_msg = strtoul( co, 0, HEX_BASE);
                if( errno) *rc = ERR_SYS_CALL;
                else
                {
                    st_hex++;
                    st_msg++;
		}
	    }

            for( ; off <= msg_len && *rc == RC_NORMAL; off++)
            {
                memcpy( conv, st_hex, 2);
                errno = 0;
                *st_msg = strtoul( co, 0, HEX_BASE);
                if( errno) *rc = ERR_SYS_CALL;
                else
                {
                    st_hex += 2;
                    st_msg++;
		}
            }

            *st_msg = '\0';
        }
    }

    *result_len = msg_len;

    return( msg);
}
