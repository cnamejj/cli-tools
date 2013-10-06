#include <strings.h>
#include "cli-sub.h"

int is_option_set( char *setting)

{
    int is_set = IS_SET_INVALID;

    if( setting) if( *setting)
    {
        if( !strcasecmp( setting, OP_IS_SET_TRUE1)) is_set = IS_SET_TRUE;
        else if( !strcasecmp( setting, OP_IS_SET_TRUE2)) is_set = IS_SET_TRUE;
        else if( !strcasecmp( setting, OP_IS_SET_TRUE3)) is_set = IS_SET_TRUE;
        else if( !strcasecmp( setting, OP_IS_SET_FALSE1)) is_set = IS_SET_FALSE;
        else if( !strcasecmp( setting, OP_IS_SET_FALSE2)) is_set = IS_SET_FALSE;
        else if( !strcasecmp( setting, OP_IS_SET_FALSE3)) is_set = IS_SET_FALSE;
        else is_set = IS_SET_INVALID;
    }

    return( is_set);
}
