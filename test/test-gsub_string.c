#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cli-sub.h"

/* --- */

int main( int narg, char **opts)

{
    int rc = 0, off, all;
    struct sub_list *patts = 0, *walk = 0;
    char *template = 0, *result = 0;

    /* --- */

    walk = patts = (struct sub_list *) malloc( sizeof *patts);

    walk->from = strdup( "%{uid}");
    walk->to = strdup( "mole");
    walk->next = (struct sub_list *) malloc( sizeof *walk);

    walk = walk->next;
    walk->from = strdup( "%{trick}");
    walk->to = strdup( "%{uid}");
    walk->next = (struct sub_list *) malloc( sizeof *walk);

    walk = walk->next;
    walk->from = strdup( "%{gid}");
    walk->to = strdup( "system");
    walk->next = (struct sub_list *) malloc( sizeof *walk);

    walk = walk->next;
    walk->from = strdup( "%{contact}");
    walk->to = strdup( "mole@no-such-domain.foo");
    walk->next = (struct sub_list *) malloc( sizeof *walk);

    walk = walk->next;
    walk->from = strdup( "%{name}");
    walk->to = strdup( "Monitor Mole");
    walk->next = 0;

    /* --- */

    all = 1;

    for( off = 1; off < narg; off++) all += strlen( opts[ off]) + 1;

    template = (char *) malloc( all);
    *template = '\0';

    strcpy( template, opts[ 1]);
    for( off = 2; off < narg; off++)
    {
        strcat( template, " ");
        strcat( template, opts[ off]);
    }

    /* --- */

    result = gsub_string( &rc, template, patts);

    printf( "From (%s)\nTo:: (%s)\n", template, result);

    /* --- */

    exit( rc);
}
