#include <string.h>
#include "err_ref.h"
#include "cli-sub.h"

/* --- */

char *combine_strings( int *rc, char *lead, char *trail)

{
    int size;
    char *combo = 0;

    if( *rc == RC_NORMAL )
    {
        size = 1;
        if( lead ) if( *lead ) size += strlen( lead );
        if( trail ) if( *trail ) size += strlen( trail );

        combo = (char *) malloc( size );
        if( combo )
        {
            *combo = '\0';
            if( lead ) strcpy( combo, lead );
            if( trail ) strcat( combo, trail );
	}
    }

    return( combo );
}

/* --- */

char *string_from_int( int *rc, int number, char *format)

{
    int outlen, sysrc;
    char *out = 0, *disp = DEF_INT_DISPLAY_FORMAT;

    if( *rc == RC_NORMAL)
    {
        if( format ) if( *format ) disp = format;

        outlen = strlen( disp ) + INT_DISPLAY_SIZE + 1;
        out = (char *) malloc( outlen );

        if( !out ) *rc = ERR_MALLOC_FAILED;
        else
        {
            sysrc = snprintf( out, outlen, disp, number );
            if( sysrc < 0 )
            {
                *rc = ERR_SYS_CALL;
                free( out );
                out = 0;
	    }
	}
    }

    return( out );
}

/* --- */

char *string_from_float( int *rc, float number, char *format)

{
    int outlen, sysrc;
    char *out = 0, *disp = DEF_FLOAT_DISPLAY_FORMAT;

    if( *rc == RC_NORMAL)
    {
        if( format ) if( *format ) disp = format;

        outlen = strlen( disp ) + FLOAT_DISPLAY_SIZE + 1;
        out = (char *) malloc( outlen );

        if( !out ) *rc = ERR_MALLOC_FAILED;
        else
        {
            sysrc = snprintf( out, outlen, disp, number );
            if( sysrc < 0 )
            {
                *rc = ERR_SYS_CALL;
                free( out );
                out = 0;
	    }
	}
    }

    return( out );
}
