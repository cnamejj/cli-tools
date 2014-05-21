#include <openssl/ssl.h>
#include <openssl/engine.h>

/* --- MOVES TO a "cli-sub.h" EVENTUALLY */

#define CTX_MODES SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER
#define MAX_READ_AHEAD 64 * 1024
#define SSL_TRUSTED_CERT_PATH "/etc/ssl/certs"

/* --- */

SSL_CTX *debug_init_ssl_context(int *sysrc, int (*callback)(int, X509_STORE_CTX *))

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
            (void) SSL_CTX_set_read_ahead(context, MAX_READ_AHEAD);
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
    *sysrc = err;

    return(context);
}

/* --- */

SSL *debug_map_sock_to_ssl(int sock, SSL_CTX *context, long (*callback)(struct bio_st *, int, const char *, int, long, long))

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
