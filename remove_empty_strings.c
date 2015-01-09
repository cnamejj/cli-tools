#include "cli-sub.h"

/* --- */

void remove_empty_strings( struct string_parts *plist )

{
    int elem, scan;

    if( !plist ) return;

    for( elem = 0; elem < plist->np; elem++ )
    {
        if( plist->list[elem] ) if( !*plist->list[elem] )
        {
            free( plist->list[elem] );
            plist->list[elem] = 0;
	}
    }

    for( elem = 0; elem < plist->np; elem++ )
    {
        for( scan = elem + 1; !plist->list[elem] && scan < plist->np; scan++ )
        {
            plist->list[elem] = plist->list[scan];
            plist->list[scan] = 0;
	}
    }

    for( elem = 0; elem < plist->np && plist->list[elem]; elem++ ) ;

    if( elem < plist->np ) plist->np = elem;

    return;
}
