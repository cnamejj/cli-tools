#include <ctype.h>

int is_reserved_uri_char( char spot)

{
    int off, check;
    static int first = 1;
    static char reserved_status[ 256];
    char *st, *unreserved_punct = "-_.!~*'()";

    if( first)
    {
        first = 0;
        st = reserved_status;
        for( off = 0; off < (sizeof reserved_status); off++)
        {
            if( isalnum( off)) *st = 0;
            else *st = 1;
            st++;
	}

        for( st = unreserved_punct; *st; st++) reserved_status[ (int) *st] = 0;
    }

    check = spot & 0xff;

    return( reserved_status[ check]);
}
