#include <stdio.h>

#include "cli-sub.h"
#include "err_ref.h"

/* --- */

void print_option_settings( FILE *out, int nflags, struct option_set *opset)

{
    int off, *int_p;
    struct option_set *co = 0;
    struct value_chain *chain = 0;

    /* Print out settings just for options included on the command line */
    fprintf( out, "Seq Num Typ Fl Opt\n");

    for( off= 0; off < nflags; off++)
    {
        co = opset + off;
        if( co->opt_num)
        {
            if( co->flags & OP_FL_REPEATS)
            {
                for( chain = (struct value_chain *) co->parsed; chain; chain = chain->next)
                {
                    fprintf( out, "%2d+ %3d %3d %2X ", off + 1, co->num, co->type, chain->flags);
                    fprintf( out, "%3d ", chain->opt_num);

                    if( co->type == OP_TYPE_INT)
                    {
                        int_p = (int *) chain->parsed;
                        fprintf( out, "%d ", *int_p);
                    }
                    else if( co->type == OP_TYPE_CHAR) fprintf( out, "(%s) ", (char *) chain->parsed);
                    else if( co->type == OP_TYPE_FLOAT) fprintf( out, "%f ", *((float *) chain->parsed));
                    else fprintf( out, "/?/ ");
                    fprintf( out, "(%s) (%s) ", co->name, chain->val);
                    fprintf( out, "\n");
		}
	    }
            else
            {
                fprintf( out, "%2d. %3d %3d %2X ", off + 1, co->num, co->type, co->flags);
                fprintf( out, "%3d ", co->opt_num);
                if( co->type == OP_TYPE_INT || co->type == OP_TYPE_FLAG)
                {
                    int_p = (int *) co->parsed;
                    fprintf( out, "%d ", *int_p);
                }
                else if( co->type == OP_TYPE_CHAR) fprintf( out, "(%s) ", (char *) co->parsed);
                else if( co->type == OP_TYPE_FLOAT) fprintf( out, "%f ", *((float *) co->parsed));
                else fprintf( out, "/?/ ");
                fprintf( out, "(%s) (%s) ", co->name, co->val);
                fprintf( out, "\n");
	    }
        }
    }

    /* Print out all options settings, includes defaults for unspecified options */
    fprintf( out, "Seq Num Typ Fl Opt\n");

    for( off= 0; off < nflags; off++)
    {
        co = opset + off;
        if(( co->flags & OP_FL_REPEATS) && co->parsed)
        {
            for( chain = (struct value_chain *) co->parsed; chain; chain = chain->next)
            {
                fprintf( out, "%2d+ %3d %3d %2X ", off + 1, co->num, co->type, chain->flags);
                fprintf( out, "%3d ", chain->opt_num);

                if( co->type == OP_TYPE_INT)
                {
                    int_p = (int *) chain->parsed;
                    fprintf( out, "%d ", *int_p);
                }
                else if( co->type == OP_TYPE_CHAR) fprintf( out, "(%s) ", (char *) chain->parsed);
                else if( co->type == OP_TYPE_FLOAT) fprintf( out, "%f ", *((float *) chain->parsed));
                else fprintf( out, "/?/ ");
                fprintf( out, "(%s) (%s) ", co->name, chain->val);
                fprintf( out, "\n");
	    }
        }
        else
        {
            fprintf( out, "%2d. %3d %3d %2X ", off + 1, co->num, co->type, co->flags);
            fprintf( out, "%3d ", co->opt_num);
            if( !co->parsed) fprintf( out, "(%s) ", co->def);
            else if( co->type == OP_TYPE_INT || co->type == OP_TYPE_FLAG)
            {
                int_p = (int *) co->parsed;
                fprintf( out, "%d ", *int_p);
            }
            else if( co->type == OP_TYPE_CHAR) fprintf( out, "(%s) ", (char *) co->parsed);
            else if( co->type == OP_TYPE_FLOAT) fprintf( out, "%f ", *((float *) co->parsed));
            else fprintf( out, "/?/ ");
            fprintf( out, "(%s) (%s) ", co->name, co->val);
            fprintf( out, "\n");
        }
    }

    return;
}
