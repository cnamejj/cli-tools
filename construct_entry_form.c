#include <string.h>
#include "cli-sub.h"
#include "err_ref.h"

/* --- */

char *construct_entry_form( char *template)

{
    int complete = 0, rc = RC_NORMAL;
    char *server_name = 0, *server_port = 0, *script_name = 0, *sep = COLON_ST, *st = 0,
      *complete_form = 0;
    struct sub_list *walk = 0, *subs = 0;

    server_name = getenv( ENV_SERVER_NAME);
    server_port = getenv( ENV_SERVER_PORT);
    script_name = getenv( ENV_SCRIPT_NAME);

    if( !server_name || !server_port || !script_name) complete = 0;
    else if( !*server_name || !*server_port || !*script_name) complete = 0;
    else
    {
        complete = 1;

        if( !strcmp( server_port, DEFAULT_HTTP_PORT))
        {
            sep = EMPTY_ST;
            server_port = EMPTY_ST;
        }
        else
        {
            st = (char *) malloc( strlen( server_port) + strlen( sep) + 1);
            if( !st) complete = 0;
            else
            {
                strcpy( st, sep);
                strcat( st, server_port);
                server_port = st;
	    }
	}
    }

    if( complete)
    {
        walk = subs = (struct sub_list *) malloc( sizeof *subs);
        if( !walk) complete = 0;

        if( complete)
        {
            walk->from = SC_FORM_PATT_SERVER;
            walk->to = server_name;
            walk->next = (struct sub_list *) malloc( sizeof *walk);
            if( !walk->next) complete = 0;
            else walk = walk->next;
	}

        if( complete)
        {
            walk->from = SC_FORM_PATT_SCRIPT;
            walk->to = script_name;
            walk->next = (struct sub_list *) malloc( sizeof *walk);
            if( !walk->next) complete = 0;
            else walk = walk->next;
	}
        
        if( complete)
        {
            walk->from = SC_FORM_PATT_PORT;
            walk->to = server_port;
            walk->next = 0;
	}
        
        if( complete)
        {
            complete_form = gsub_string( &rc, template, subs);
            if( rc != RC_NORMAL) complete = 0;
	}
    }

    if( st) free( st);

    if( !complete && complete_form) free( complete_form);

    for( walk = subs; !walk; )
    {
        subs = walk->next;
        free( walk);
        walk = subs;
    }

    /* --- */

    return( complete_form);
}
