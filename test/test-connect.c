#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "../cli-sub.h"

int main(int narg, char **opts)

{
    int port = 8080, timeout = 30000, sock, rc = 0;
    char *host = 0, *st;
    char http_req[8192], http_response[8192];

    memset(http_req, '\0', sizeof http_req);

    strcat(http_req, "GET / HTTP/1.0\r\n\r\n");

    if(narg < 2) { printf( "Err: No hostname\n"); exit(1); }

    if(narg >= 3) port = atoi(opts[2]);

    if(narg >= 4) timeout = atoi(opts[3]);

    host = opts[1];

    sock = connect_host(&rc, host, port, timeout, AF_INET);

    printf( "After: sock=%d rc=%d\n", sock, rc);

    errno = 0;
    rc = write(sock, http_req, strlen(http_req));
    printf( "dbg:: post-write rc=%d errno=%d '%s'\n", rc, errno, strerror(errno));

    sleep(2);
  
    for( rc = read(sock, http_response, sizeof http_response); rc > 0; rc = read(sock, http_response, sizeof http_response))
    {
        st = http_response;
        printf( "Got:\n%s\n", st);
        sleep(1);
    }

    sleep(1);

    exit(0);
}
