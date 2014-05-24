#include <errno.h>

#include "http-fetch.h"

#define EMSG_SIZE 128

/* --- */

int stash_ssl_err_info( struct fetch_status *fetch, unsigned long sslerr)

{
    int ret = 0;
    char *emsg;

    if( sslerr)
    {
        ret = sslerr;
        fetch->end_errno = ret;
        emsg = (char *) malloc( EMSG_SIZE);
        if( emsg)
        {
            ERR_error_string_n( sslerr, emsg, EMSG_SIZE);
            fetch->err_msg = emsg;
	}
        else fetch->err_msg = 0;
    }
    else
    {
        ret = errno;
        fetch->end_errno = ret;
    }

    return( ret);
}
