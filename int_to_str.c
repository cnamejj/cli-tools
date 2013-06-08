#include <stdio.h>

#include "cli-sub.h"

/* --- */

char *int_to_str( char *buff, int buff_len, int source, char *format)

{
    static char def_result[1] = { '\0' };
    char *def_format = "%d", *use_format = 0, *result = 0;


    use_format = format;
    if( !use_format) use_format = def_format;
    else if( !*use_format) use_format = def_format;

    if( buff && buff_len)
    {
        snprintf( buff, buff_len, use_format, source);
        result = buff;
    }
    else result = def_result;

    return( result);
}
