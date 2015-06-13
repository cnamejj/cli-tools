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

struct match_list {
    struct sub_list *patt;
    char *ref;
};

#define NO_PATT_MATCH -1

/* --- */

char *gsub_string( int *rc, char *template, struct sub_list *patts)

{
    int final_len = 0, backlen = 0, max_patt_len = 0, n_patt = 0, lrc = RC_NORMAL,
      seq, next_seq, asis = 0;
    char *next_sub = 0, *remaining = 0, *end_of_template = 0, *eos = 0, *result = 0,
      *check_sub, *blockpoint = 0, blockhold = '\0', *last_possible = 0, *next_loc = 0;
int dbg_seq;
char dbg_buff[256], *dbg_last = 0;
    struct sub_list *walk = 0, *curr_patt = 0, *next_patt = 0;
    struct match_list *plist = 0;

    ENTER( "gsub_string" )

    end_of_template = template + strlen( template);

    for( walk = patts; walk; walk = walk->next )
    {
        n_patt++;
        if( strlen(walk->from) > max_patt_len ) max_patt_len = strlen(walk->from);
    }

    plist = (struct match_list *) malloc( n_patt * (sizeof *plist) );
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
        /* --- */
#ifdef INCLUDE_UGLY_DEBUG
        {
            int dbg_pcount = 0, dbg_ll = 0;
            char *dbg_st = 0;
            for( walk = patts; walk; walk = walk->next) dbg_pcount++;
            fprintf(stderr, "dbg:: gsub-string: template(%ld) #patts(%d)\n", strlen(template), dbg_pcount);

            fprintf(stderr, "\n- - - - -\n");
            for(dbg_st = template; *dbg_st; dbg_st++)
            {
                fputc(*dbg_st, stderr);
                if(*dbg_st == '\n') dbg_ll = 0;
                else if(dbg_ll < 80) dbg_ll++;
                else
                {
                    fputc('\n', stderr);
                    dbg_ll = 0;
                }
            }
            fprintf(stderr, "\n- - - -\n");

            dbg_pcount = 0;
            for( walk = patts; walk; walk = walk->next)
            {
                dbg_pcount++;
                fprintf(stderr, "dbg:: gsub-string: p#%d f(%s) t(%s)\n", dbg_pcount, walk->from, walk->to);
            }
        }
#endif

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

dbg_last = result + final_len;
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
*eos = '\0';
// fprintf(stderr, "dbg:: gsub_string: build: asis %d r(%s)\n", asis, result);
// if(remaining + asis > dbg_last) fprintf(stderr, "dbg:: gsub_string: asis over-run %x > %x\n", remaining + asis, dbg_last);
		}

                if( next_seq == NO_PATT_MATCH ) remaining = 0;
                else
                {
                    strcpy( eos, next_patt->to);
// if(eos + strlen(next_patt->to) > dbg_last) fprintf(stderr, "dbg:: gsub_string: subs over-run %x > %x f(%s) t(%s) r(%s) p(%s)\n", eos + strlen(next_patt->to), dbg_last, 
//  next_patt->from, next_patt->to, eos, template);
                    eos += strlen( next_patt->to);
                    remaining = next_loc + strlen(next_patt->from);
// if(remaining > dbg_last) fprintf(stderr, "dbg:: gsub_string: incr over-run %x > %x\n", remaining, dbg_last);
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
