#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* --- */

#define BASE_MAP( HEX, BASE) \
    base_letter_map[  0x ## HEX] = BASE ;

#define EXT_INTRO 0XC3

#define TRANS 1
#define OPEN 0

/* --- */

int main( int narg, char **opts)

{
    int off, ch, rc, status = OPEN;
    unsigned char base_letter_map[ 256];

    for( off = 0; off < (sizeof base_letter_map); off++) base_letter_map[ off] = ' ';

    BASE_MAP( A1, 'a')
    BASE_MAP( AD, 'i')
    BASE_MAP( B1, 'n')
    BASE_MAP( B3, 'o')
    BASE_MAP( BC, 'u')

    /* --- */

    for( ; (ch = fgetc( stdin)) != EOF; )
    {
        if( status == TRANS)
        {
            rc = fputc( base_letter_map[ ch], stdout);
            status = OPEN;
	}
        else if( ch == EXT_INTRO)
        {
            status = TRANS;
            rc = 0;
	}
        else rc = fputc( ch, stdout);

        if( rc == EOF)
        {
            fprintf( stderr, "Error, write failed! rc=%d\b", errno);
            exit( 1);
	}
    }

    exit( 0);
}
