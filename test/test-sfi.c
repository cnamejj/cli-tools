#include "../cli-sub.h"

int main(int narg, char **opts)

{
    int rc = 0, number = -1;
    char *format = 0, *out, *st, *st2;

    if(narg >= 2) number = atoi(opts[1]);

    if(narg >= 3) format = opts[2];

    out = string_from_int( &rc, number, format);

    if( format) st = format;
    else st = "N/A";

    if( out) st2 = out;
    else st2 = "N/A";

    fprintf( stdout, "rc=%d num=%d format(%s) out(%s)\n", rc, number, st, st2);

    exit(0);
}
