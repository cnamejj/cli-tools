#include <openssl/ssl.h>

#include "cli-sub.h"
#include "http-fetch.h"
#include "err_ref.h"

/* --- */

void setup_ssl_env( int *rc, struct plan_data *plan)

{
    struct target_info *target = 0;
    struct fetch_status *fetch = 0;

    if( *rc == RC_NORMAL)
    {
        target = plan->target;
        fetch = plan->status;

        if( !target || !fetch) *rc = ERR_UNSUPPORTED;
        else if(target->use_ssl && !fetch->ssl_context)
        {
            fetch->ssl_context = init_ssl_context( verify_ssl_callback);
            if( !fetch->ssl_context)
            {
                *rc = ERR_SSL_INIT_FAILED;
                (void) stash_ssl_err_info( fetch, ERR_peek_error());
	    }
	}
    }

    return;
}
