#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "err_ref.h"
#include "net-task-data.h"

/* temporary to get debug malloc info... */
#include "cli-sub.h"

/* --- */

int parse_destination_value( struct task_details *plan, char *destination)

{
    int rc = RC_NORMAL, hostlen, port, errlen;
    char *st = 0, *last_colon, *invalid = 0, *host = 0;

    /* Save the address of the last colon found, which we need to split the
     * hostname (or IP address) from the port.
     */
    for( st = destination, last_colon = 0; *st; st++)
      if( *st == ':') last_colon = st;

    if( last_colon > destination)
    {
        hostlen = last_colon - destination;
        host = (char *) malloc( hostlen + 1);
        if( !host) rc = ERR_MALLOC_FAILED;
        else
        {
            memcpy( host, destination, hostlen);
            *(host + hostlen) = '\0';
            st = last_colon + 1;
            if( !*st) rc = ERR_INVALID_DATA;
            else
            {
                port = strtol( st, &invalid, 10);
                if( *invalid) rc = ERR_INVALID_DATA;
	    }
	}
    }
    else rc = ERR_INVALID_DATA;

    if( rc == RC_NORMAL)
    {
        plan->target_host = host;
        plan->target_port = port;
    }
    else if( rc == ERR_INVALID_DATA)
    {
        errlen = strlen( ERRMSG_BAD_DEST) + strlen( destination);
        plan->err_msg = (char *) malloc( errlen);
        if( !plan->err_msg) rc = ERR_MALLOC_FAILED;
        else snprintf( plan->err_msg, errlen, ERRMSG_BAD_DEST, destination);
    }
    
    return( rc);
}
