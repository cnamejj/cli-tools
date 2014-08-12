#include <openssl/ssl.h>
#include <openssl/engine.h>

#include "cli-sub.h"

/* --- */

SSL_CTX *init_ssl_context(int (*callback)(int, X509_STORE_CTX *))

{
    int err = 0, rc;
    SSL_CTX *context = 0;
    const SSL_METHOD *meth = 0;

    SSL_load_error_strings();
    SSL_library_init();
    ENGINE_load_builtin_engines();

    meth = SSLv23_client_method();
    context = SSL_CTX_new(meth);
    if(!context) err = 1;
    else
    {
        rc = SSL_CTX_set_cipher_list(context, "ALL");
        if(!rc) err = 1;
        else
        {
            (void) SSL_CTX_set_mode(context, CTX_MODES);
            (void) SSL_CTX_set_read_ahead(context, SSL_MAX_READ_AHEAD);
            (void) SSL_CTX_set_options(context, SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER);
            rc = SSL_CTX_load_verify_locations(context, 0, SSL_TRUSTED_CERT_PATH);
            if(!rc) err = 1;
            else if(callback) SSL_CTX_set_verify(context, SSL_VERIFY_PEER, callback);
	}
        
    }

    if(err && context)
    {
        SSL_CTX_free(context);
        context = 0;
    }

    return(context);
}

/* --- */

SSL *map_sock_to_ssl(int sock, SSL_CTX *context, long (*callback)(struct bio_st *, int, const char *, int, long, long))

{
    int err = 0, rc;
    SSL *ssl = 0;

    ssl = SSL_new(context);
    if(!ssl) err = 1;
    else
    {
        rc = SSL_set_fd(ssl, sock);
        if(!rc) err = 1;
        else
        {
            SSL_set_connect_state(ssl);

            if(callback)
            {
                BIO_set_callback(ssl->rbio, callback);
                BIO_set_callback(ssl->wbio, callback);
            }
        }
    }

    if(err && ssl)
    {
        SSL_free(ssl);
        ssl = 0;
    }

    return(ssl);
}
