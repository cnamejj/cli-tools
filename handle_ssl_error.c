#include <errno.h>

#include "cli-sub.h"
#include "http-fetch.h"
#include "err_ref.h"

/* --- */

int handle_ssl_error( int *sslact, SSL *ssl, int io_rc, int sock, int max_wait)

{
    int rc = RC_NORMAL, sysrc, event, err;

    err = SSL_get_error( ssl, io_rc);

// if(err==SSL_ERROR_WANT_READ) printf("dbg:: HSE, err=want-read");
// else if(err==SSL_ERROR_WANT_WRITE) printf("dbg:: HSE, err=want-write");
// else if(err==SSL_ERROR_WANT_CONNECT) printf("dbg:: HSE, err=want-connect");
// else if(err==SSL_ERROR_WANT_ACCEPT) printf("dbg:: HSE, err=want-accept");
// else if(err==SSL_ERROR_WANT_X509_LOOKUP) printf("dbg:: HSE, err=want-X509");
// else if(err==SSL_ERROR_SYSCALL) printf("dbg:: HSE, err=syscall");
// else if(err ==SSL_ERROR_ZERO_RETURN) printf("dbg:: HSE, err=zero");
// else if(err ==SSL_ERROR_SSL) printf("dbg:: HSE, err=ssl");
// else printf("dbg:: HSE, err=other");
// printf(" s:%d e:%d\n", err, errno);

    if( err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
    {
        if( err == SSL_ERROR_WANT_READ) event = POLL_EVENTS_READ;
        else event = POLL_EVENTS_WRITE;

        sysrc = wait_until_sock_ready( sock, event, max_wait);
        if( sysrc > 0 && err == SSL_ERROR_WANT_WRITE) *sslact = SSLACT_WRITE;
        else if( sysrc > 0 && err == SSL_ERROR_WANT_READ) *sslact = SSLACT_READ;
        else
        {
            *sslact = SSLACT_ERR_FATAL;
            if( !sysrc) rc = ERR_POLL_TIMEOUT;
            else rc = ERR_POLL_FAILED;
	}
    }

    else if( err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT
      || err == SSL_ERROR_WANT_X509_LOOKUP) *sslact = SSLACT_RETRY;

    else if( err == SSL_ERROR_SYSCALL)
    {
        err = errno;
        if( err == EAGAIN || err == EBUSY || err == EINTR
          || err == EWOULDBLOCK || err == EINPROGRESS
          || err == ERESTART) *sslact = SSLACT_RETRY;
        else
        {
            rc = ERR_SYS_CALL;
            *sslact = SSLACT_ERR_HANDSHAKE;
	}
    }

    else if( err == SSL_ERROR_ZERO_RETURN)
    {
        rc = ERR_SSL_EOS;
        *sslact = SSLACT_ERR_CLOSED;
    }

    else
    {
        rc = ERR_SSL_ERROR;
        *sslact = SSLACT_ERR_FATAL;
    }

// printf("dbg:: HSE: leaving with rc=%d\n", rc);

    return( rc);    
}
