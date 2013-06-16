#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* --- */

#define BASE_MAP( HEX, BASE) \
    base_letter_map[  0x ## HEX] = BASE ;

/* --- */

int main( int narg, char **opts)

{
    int off, ch, rc;
    unsigned char base_letter_map[ 256];

    for( off = 0; off < 128; off++) base_letter_map[ off] = '\0';
    for( off = 128; off < (sizeof base_letter_map); off++) base_letter_map[ off] = ' ';

    BASE_MAP( A1, 'i')
    BASE_MAP( A2, 'C')
    BASE_MAP( C0, 'A')
    BASE_MAP( C1, 'A')
    BASE_MAP( C2, 'A')
    BASE_MAP( C3, 'A')
    BASE_MAP( C4, 'A')
    BASE_MAP( C5, 'A')
    BASE_MAP( C6, 'A')
    BASE_MAP( C7, 'C')
    BASE_MAP( C8, 'E')
    BASE_MAP( C9, 'E')
    BASE_MAP( CA, 'E')
    BASE_MAP( CB, 'E')
    BASE_MAP( CC, 'I')
    BASE_MAP( CD, 'I')
    BASE_MAP( CE, 'I')
    BASE_MAP( CF, 'I')
    BASE_MAP( D0, 'D')
    BASE_MAP( D1, 'N')
    BASE_MAP( D2, 'O')
    BASE_MAP( D3, 'O')
    BASE_MAP( D4, 'O')
    BASE_MAP( D5, 'O')
    BASE_MAP( D6, 'O')
    BASE_MAP( D8, 'O')
    BASE_MAP( D9, 'U')
    BASE_MAP( DA, 'U')
    BASE_MAP( DB, 'U')
    BASE_MAP( DC, 'U')
    BASE_MAP( DD, 'Y')
    BASE_MAP( E0, 'a')
    BASE_MAP( E1, 'a')
    BASE_MAP( E2, 'a')
    BASE_MAP( E3, 'a')
    BASE_MAP( E4, 'a')
    BASE_MAP( E5, 'a')
    BASE_MAP( E6, 'a')
    BASE_MAP( E7, 'c')
    BASE_MAP( E8, 'e')
    BASE_MAP( E9, 'e')
    BASE_MAP( EA, 'e')
    BASE_MAP( EB, 'e')
    BASE_MAP( EC, 'i')
    BASE_MAP( ED, 'i')
    BASE_MAP( EE, 'i')
    BASE_MAP( EF, 'i')
    BASE_MAP( F1, 'n')
    BASE_MAP( F2, 'o')
    BASE_MAP( F3, 'o')
    BASE_MAP( F4, 'o')
    BASE_MAP( F5, 'o')
    BASE_MAP( F6, 'o')
    BASE_MAP( F8, 'o')
    BASE_MAP( F9, 'u')
    BASE_MAP( FA, 'u')
    BASE_MAP( FB, 'u')
    BASE_MAP( FC, 'u')
    BASE_MAP( FD, 'y')
    BASE_MAP( FF, 'y')

    /* --- */

    for( ; (ch = fgetc( stdin)) != EOF; )
    {
        if( base_letter_map[ ch] != '\0') rc = fputc( base_letter_map[ ch], stdout);
        else rc = fputc( ch, stdout);

        if( rc == EOF)
        {
            fprintf( stderr, "Error, write failed! rc=%d\b", errno);
            exit( 1);
	}
    }

    exit( 0);
}
