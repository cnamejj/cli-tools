#include <errno.h>

#include "cli-sub.h"
#include "http-fetch.h"
#include "err_ref.h"

/* --- */

int handle_ssl_error( int *sslerr, SSL *ssl, int io_rc, int sock, int max_wait)

{
    int rc = RC_NORMAL, sysrc, event, err;

    err = SSL_get_error( ssl, io_rc);

    if( err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
    {
        if( err == SSL_ERROR_WANT_READ) event = POLL_EVENTS_READ;
        else event = POLL_EVENTS_WRITE;

        sysrc = wait_until_sock_ready( sock, event, max_wait);
        if( sysrc > 0) *sslerr = SSLERR_NORMAL;
        else
        {
            *sslerr = SSLERR_FATAL;
            if( !sysrc) rc = ERR_POLL_TIMEOUT;
            else *sslerr = SSLERR_FATAL;
	}
    }

    else if( err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT
      || err == SSL_ERROR_WANT_X509_LOOKUP) *sslerr = SSLERR_RETRY;

    else if( err == SSL_ERROR_SYSCALL)
    {
        err = errno;
        if( err == EAGAIN || err == EBUSY || err == EINTR
          || err == EWOULDBLOCK || err == EINPROGRESS
          || err == ERESTART) *sslerr = SSLERR_RETRY;
        else
        {
            rc = ERR_SYS_CALL;
            *sslerr = SSLERR_HANDSHAKE;
	}
    }

    else if( err == SSL_ERROR_ZERO_RETURN)
    {
        rc = ERR_SSL_EOS;
        *sslerr = SSLERR_CLOSED;
    }

    else
    {
        rc = ERR_SSL_ERROR;
        *sslerr = SSLERR_FATAL;
    }

    return( rc);    
}
