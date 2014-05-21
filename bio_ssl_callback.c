#include <openssl/ssl.h>

/* stub routine for now... */


long bio_ssl_callback(BIO *bn, int flags, const char *buff, int blen, long ignore, long ret)

{
    return( ret);
}
