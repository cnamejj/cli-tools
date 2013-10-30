#include <string.h>
#include "cli-sub.h"

char *dup_memory( char *st_pos, char *en_pos)

{
    int size;
    char *string = 0;

    if( st_pos && en_pos) if( en_pos >= st_pos)
    {
        size = (en_pos - st_pos) + 1;
        string = (char *) malloc( size + 1);
        if( string)
        {
            memcpy( string, st_pos, size);
            *(string + size) = EOS_CH;
        }
    }

    return( string);
}
