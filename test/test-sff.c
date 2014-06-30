#include "../cli-sub.h"

int main(int narg, char **opts)

{
    int rc = 0;
    float number = -1.2;
    char *format = 0, *alt = 0, *out = 0, *out2 = 0, *combo = 0;

    if(narg >= 2) number = strtof(opts[1], 0);
    if(narg >= 3) format = opts[2];
    if(narg >= 4) alt = opts[3];

    out = string_from_float( &rc, number, format);

    out2 = string_from_float( &rc, number, alt);

    combo = combine_strings( &rc, out, out2);

    if( !format) format = "N/A";
    if( !alt) alt = "N/A";
    if( !out) out = "N/A";
    if( !out2) out2 = "N/A";
    if( !combo) combo = "Error";

    fprintf( stdout, "rc=%d num=%f format(%s) alt(%s) out(%s) out2(%s) combo(%s)\n", rc, number, format, alt, out, out2, combo);

    exit(0);
}
