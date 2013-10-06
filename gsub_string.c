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
    int final_len = 0, delta = 0, backlen = 0;
    char *next_sub = 0, *remaining = 0, *end_of_template = 0, *eos = 0, *result = 0,
      *check_sub;
    struct sub_list *walk = 0, *curr_patt = 0;

    end_of_template = template + strlen( template);

    /* --- */

    for( remaining = template; remaining; )
    {
        next_sub = end_of_template;

        for( walk = patts; walk; walk = walk->next)
        {
            check_sub = strstr( remaining, walk->from);
            if( check_sub && check_sub < next_sub)
            {
                curr_patt = walk;
                next_sub = check_sub;
	    }
	}

        if( next_sub != end_of_template)
        {
            delta += strlen( curr_patt->to) - strlen( curr_patt->from);
            remaining = next_sub + strlen( curr_patt->from);
	}
        else remaining = 0;
    }

    /* --- */

    final_len = strlen( template) + delta;

    result = (char *) malloc( final_len + 1);
    if( !result) *rc = ERR_MALLOC_FAILED;
    else
    {
        eos = result;
        *eos = '\0';

        for( remaining = template; remaining; )
        {
            next_sub = end_of_template;

            for( walk = patts; walk; walk = walk->next)
            {
                check_sub = strstr( remaining, walk->from);
                if( check_sub && check_sub < next_sub)
                {
                    curr_patt = walk;
                    next_sub = check_sub;
                }
            }

            if( next_sub != end_of_template)
            {
                backlen = next_sub - remaining;
                strncpy( eos, remaining, backlen);
                eos += backlen;

                strcpy( eos, curr_patt->to);
                eos += strlen( curr_patt->to);

                remaining = next_sub + strlen( curr_patt->from);
            }
            else
            {
                strcpy( eos, remaining);
                remaining = 0;
            }
        }
    }

    /* --- */

    return( result);
}
