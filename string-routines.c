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

/* --- */

char *string_from_clock( int *rc, time_t number, char *format)

{
    int outlen;
    char *out = 0, *disp = DEF_FLOAT_DISPLAY_FORMAT;
    struct tm when;

    if( *rc == RC_NORMAL)
    {
        if( format ) if( *format ) disp = format;

        outlen = strlen( disp ) + CLOCK_DISPLAY_SIZE + 1;
        out = (char *) malloc( outlen );

        if( !out ) *rc = ERR_MALLOC_FAILED;
        else
        {
            if( !localtime_r( &number, &when ) ) *rc = ERR_SYS_CALL;
            else if( !strftime( out, outlen, disp, &when ) )
              *rc = ERR_SYS_CALL;

            if( *rc != RC_NORMAL )
            {
                free( out );
                out = 0;
            }
	}
    }

    return( out );
}

/* --- */

char *string_from_typed_format( int *rc, void *number, int dtype, char *format )

{
    char *out = 0;

    if( dtype == DTYPE_TIME ) out = string_from_clock( rc, *((time_t *) number), format );
    else if( dtype == DTYPE_FIXED ) out = string_from_int( rc, *((int *) number), format );
    else if( dtype == DTYPE_FLOAT ) out = string_from_float( rc, *((float *) number), format );
    else out = string_from_float( rc, *((float *) number), format ); /* have to try something */

    return( out );
}

