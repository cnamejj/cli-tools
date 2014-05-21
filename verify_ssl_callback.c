#include <openssl/ssl.h>

/* stub routine for now... */

int verify_ssl_callback(int ok, X509_STORE_CTX *context)

{
    return ok;
}
