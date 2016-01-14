#include "http-fetch.h"

/* --- */

int verify_sni_callback( SSL *ssl, int *alert, void *args)

{
/*... stubbed out for now, should pull the hostname and compare to what we wanted ...*/

    return SSL_TLSEXT_ERR_OK;
}
