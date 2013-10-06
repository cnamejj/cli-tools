#include <stdlib.h>

int called_as_cgi()

{
    int rc = 0, off;
    char *cgi_env_var[] = { "DOCUMENT_ROOT", "GATEWAY_INTERFACE", "HTTP_HOST",
      "PATH_INFO", "QUERY_STRING", "REQUEST_METHOD", "SERVER_ADDR",
      "SERVER_NAME", "SERVER_PROTOCOL", "SERVER_SOFTWARE" };

    for( off=0; !rc && off < (sizeof cgi_env_var) / (sizeof *cgi_env_var); off++)
     rc = !!getenv( cgi_env_var[ off]);

    return rc;
}
