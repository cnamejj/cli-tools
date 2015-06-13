/*
 *
 * Todo
 * ----
 * - Test to make sure the code handles cases where one sub-pattern creates
 * - output that matches another sub-pattern, possibly causing a double substition
 * - (and blowing up the final length assumptions).
 */

#include <string.h>
#include <stdlib.h>

#include "cli-sub.h"
#include "err_ref.h"

/* --- */

char *gsub_string( int *rc, char *template, struct sub_list *patts)

{
    int final_len = 0, max_patt_len = 0, n_patt = 0, lrc = RC_NORMAL, seq,
       next_seq, asis = 0;
    char *remaining = 0, *end_of_template = 0, *eos = 0, *result = 0, *next_loc = 0;
    struct sub_list *walk = 0, *next_patt = 0;
    struct sub_list_reference *plist = 0;

    ENTER( "gsub_string" )

    end_of_template = template + strlen( template);

    for( walk = patts; walk; walk = walk->next )
    {
        n_patt++;
        if( strlen(walk->from) > max_patt_len ) max_patt_len = strlen(walk->from);
    }

    plist = (struct sub_list_reference *) malloc( n_patt * (sizeof *plist) );
    if( !plist ) lrc = ERR_MALLOC_FAILED;
    else
    {
        for( seq = 0, walk = patts; seq < n_patt; seq++ )
        {
            plist[seq].patt = walk;
            plist[seq].ref = 0;
            walk = walk->next;
	}
    }

    if( lrc == RC_NORMAL )
    {
        /* Caculate how large the template will be after all the substitutions */

        final_len = strlen(template);

        for( remaining = template; remaining; )
        {
            next_loc = end_of_template;
            next_seq = NO_PATT_MATCH;

            for( seq = 0; seq < n_patt; seq++ ) if( plist[seq].patt )
            {
                if( plist[seq].ref < remaining )
                {
                    plist[seq].ref = strstr( remaining, plist[seq].patt->from );
                    if( !plist[seq].ref ) plist[seq].patt = 0;
                }

                if( plist[seq].ref && plist[seq].ref < next_loc )
                {
                    next_loc = plist[seq].ref;
                    next_patt = plist[seq].patt;
                    next_seq = seq;
		}
	    }

            if( next_seq == NO_PATT_MATCH ) remaining = 0;
            else
            {
                remaining = next_loc + strlen(next_patt->from);
                final_len += strlen(next_patt->to) - strlen(next_patt->from);
                plist[next_seq].ref = 0;
	    }
	}

        /* --- */

        INSUB( "gsub_string", "before-rebuild" )

        result = (char *) malloc( final_len + 1);
        if( !result) lrc = ERR_MALLOC_FAILED;
        else
        {
            eos = result;
            *eos = '\0';

            for( seq = 0, walk = patts; seq < n_patt; seq++ )
            {
                plist[seq].patt = walk;
                plist[seq].ref = 0;
                walk = walk->next;
            }

            for( remaining = template; remaining; )
            {
                next_loc = end_of_template;
                next_seq = NO_PATT_MATCH;

                for( seq = 0; seq < n_patt; seq++ ) if( plist[seq].patt )
                {
                    if( plist[seq].ref < remaining )
                    {
                        plist[seq].ref = strstr( remaining, plist[seq].patt->from );
                        if( !plist[seq].ref ) plist[seq].patt = 0;
                    }

                    if( plist[seq].ref && plist[seq].ref < next_loc )
                    {
                        next_loc = plist[seq].ref;
                        next_patt = plist[seq].patt;
                        next_seq = seq;
		    }
	        }

                asis = next_loc - remaining;
                if( asis)
                {
                    strncpy( eos, remaining, asis );
                    eos += asis;
		}

                if( next_seq == NO_PATT_MATCH ) remaining = 0;
                else
                {
                    strcpy( eos, next_patt->to);
                    eos += strlen( next_patt->to);
                    remaining = next_loc + strlen(next_patt->from);
                    plist[next_seq].ref = 0;
		}
	    }

            *eos = '\0';
        }
    }

    /* --- */

    if( plist) free( plist);

    if( lrc != RC_NORMAL) *rc = lrc;

    /* --- */

    LEAVE( "gsub_string" )

    return( result);
}
