#define SHOW_DEBUG 1

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "../cli-sub.h"
#include "../err_ref.h"
#include "tls/s2n_connection.h"

/* --- */

#define BUFFER_SIZE 256 * 1024
            
#define DEF_PORT 443
#define DEF_URI "/"

#define IS_ONE_STR "1"

#define SN_ENABLE_CLIENT "S2N_ENABLE_CLIENT_MODE"
#define SN_DISABLE_MLOCK "S2N_DONT_MLOCK"

#define MAX_CONNECT_TIME 10 * 1000
#define MAX_IO_WAIT 4 * 1000

#define ERR_SETUP_FAIL 101
#define ERR_SOCKET_FAIL 102
#define ERR_NO_HOSTNAME 103
#define ERR_LOOKUP_FAIL 104
#define ERR_SOCKOPT_FAIL 105
#define ERR_CONNECT_FAIL 106
#define ERR_REQ_PORT_FAIL 107
#define ERR_NEGOTIATE_FAIL 108
#define ERR_READ_FAIL 109
#define ERR_WRITE_FAIL 110
#define ERR_MALLOC_FAIL 111

#define HTTP_REQ_TEMPLATE "GET %s HTTP/1.1\r\n\
Host: %s\r\n\
User-Agent: Mozilla/5.0 Gecko/20100101 Firefox/23.0\r\n\
Accept: */*\r\n\
Connection: close\r\n\
DNT: 1\r\n\r\n"

/* --- */

#define ERR_EXIT( RC, MSG ) \
{ \
    fprintf( stderr, "Err: rc=%d, errno=%d, %s / %s\n", \
      RC, errno, MSG, strerror( errno ) ); \
    exit( 1); \
}

/* --- */

int connect_to_server(char *hostname, int port);
struct s2n_connection *exec_ssl_trans( int sock, char *hostname, char *uri);

/* --- */

int main( int narg, char **opts )

{
    int rc = RC_NORMAL, sysrc, sock, port;
    char *hostname = 0, *uri;
    struct s2n_connection *tls_conn;
    s2n_blocked_status blocked;
    
    /* --- */

    if( narg < 2 ) ERR_EXIT( ERR_NO_HOSTNAME, "No hostname given on command line" )
    hostname = opts[1];
    
    if( narg >= 3 )
    {
        port = strtoul(opts[2], 0, 10);
        if( !port || errno == ERANGE ) ERR_EXIT( ERR_REQ_PORT_FAIL, "Can't convert the requested port to a number" )
    }
    else port = DEF_PORT;

    if( narg >= 4 ) uri = opts[3];
    else uri = DEF_URI;

    /* --- */

    sysrc = setenv(SN_ENABLE_CLIENT, IS_ONE_STR, 1);
    if( !sysrc ) sysrc = setenv(SN_DISABLE_MLOCK, IS_ONE_STR, 1);
    if( !sysrc ) sysrc = s2n_init();
    if( sysrc ) ERR_EXIT( ERR_SETUP_FAIL, "Setup failed" )

    /* --- */

    printf("\n- - - Req #1");
    sock = connect_to_server( hostname, port);
    tls_conn = exec_ssl_trans( sock, hostname, uri);

    s2n_shutdown(tls_conn, &blocked);
    s2n_connection_free(tls_conn);

    printf("\n- - - Req #2");
    sock = connect_to_server( hostname, port);
    tls_conn = exec_ssl_trans( sock, hostname, uri);

    s2n_shutdown(tls_conn, &blocked);
    s2n_connection_free(tls_conn);

    /* --- */

    return rc;
}

/* --- */

int connect_to_server(char *hostname, int port)

{
    int sysrc, sock;
    struct addrinfo hints, *host_recs = 0, *ai = 0;
    struct sockaddr_in sa, *saddr4 = 0;
    struct linger linger;
    struct pollfd netbox;

    sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if( sock == -1 ) ERR_EXIT( ERR_SOCKET_FAIL, "Call to socket() failed" )

    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = 0;
    hints.ai_canonname = 0;
    hints.ai_next = 0;

    sysrc = getaddrinfo(hostname, NULL, &hints, &host_recs);
    if( sysrc ) ERR_EXIT( ERR_LOOKUP_FAIL, "DNS lookup failed" )

    for( ai = host_recs; ai; ai = ai->ai_next )
      if( ai->ai_family == AF_INET )
        saddr4 = (struct sockaddr_in *) ai->ai_addr;

    if( !saddr4 ) ERR_EXIT( ERR_LOOKUP_FAIL, "No IPv4 records found in result set" )
    memcpy(&sa, saddr4, sizeof sa);

    linger.l_onoff = 1;
    linger.l_linger = 300;
    sysrc = setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger, sizeof linger);
    if( sysrc ) ERR_EXIT( ERR_SOCKOPT_FAIL, "Can't set 'linger' option on socket'" )

    /* --- */

    sa.sin_port = htons(port);
    sysrc = connect(sock, (struct sockaddr *) &sa, sizeof sa);
    if( !sysrc || errno == EINPROGRESS )
    {
        netbox.fd = sock;
        netbox.events = POLL_EVENTS_WRITE;
        netbox.revents = 0;

        sysrc = poll(&netbox, 1, MAX_CONNECT_TIME);
        if( !sysrc ) ERR_EXIT( ERR_CONNECT_FAIL, "Timed out trying to connect to server" )
        if( sysrc == -1 ) ERR_EXIT( ERR_CONNECT_FAIL, "Can't connect to server" )
    }
    else ERR_EXIT( ERR_CONNECT_FAIL, "Call to connect() failed" )
    
    return(sock);
}

/* --- */

struct s2n_connection *exec_ssl_trans( int sock, char *hostname, char *uri)

{
    int sysrc, neg_try, req_len, buff_len = BUFFER_SIZE, done;
    char buff[BUFFER_SIZE], *req = 0;
    struct s2n_connection *tls_conn;
    struct pollfd netbox;
    struct s2n_config *config;
    s2n_blocked_status blocked;

    config = s2n_config_new();
    s2n_config_set_cipher_preferences(config, "default");
    tls_conn = s2n_connection_new(S2N_CLIENT);

    s2n_connection_set_fd(tls_conn, sock);

    for( neg_try = 1; neg_try; )
    {
        sysrc = s2n_negotiate(tls_conn, &blocked);

        if( blocked == S2N_BLOCKED_ON_READ )
        {
            netbox.fd = sock;
            netbox.events = POLL_EVENTS_READ;
            netbox.revents = 0;

            sysrc = poll(&netbox, 1, MAX_IO_WAIT);
            if( !sysrc ) ERR_EXIT( ERR_READ_FAIL, "Timed out waiting to receive data" )
            if( sysrc == -1 ) ERR_EXIT( ERR_READ_FAIL, "Can't read from socket" )
	}

        else if( blocked == S2N_BLOCKED_ON_WRITE )
        {
            netbox.fd = sock;
            netbox.events = POLL_EVENTS_WRITE;
            netbox.revents = 0;

            sysrc = poll(&netbox, 1, MAX_IO_WAIT);
            if( !sysrc ) ERR_EXIT( ERR_WRITE_FAIL, "Timed out waiting to receive data" )
            if( sysrc == -1 ) ERR_EXIT( ERR_WRITE_FAIL, "Can't read from socket" )
	}

        else
        {
            neg_try = 0;
            if( sysrc ) ERR_EXIT( ERR_NEGOTIATE_FAIL, "Can't negotiate protocols" )
	}
    }

#ifdef SHOW_DEBUG
    printf( "dbg:: Procotol '%s'\n", s2n_get_application_protocol(tls_conn));
#endif

    /* --- */

    req_len = (sizeof HTTP_REQ_TEMPLATE) + strlen(hostname) + strlen(uri);
    req = (char *) malloc(req_len);
    if( !req ) ERR_EXIT( ERR_MALLOC_FAIL, "Can't allocate space for HTTP request" )
    snprintf(req, req_len, HTTP_REQ_TEMPLATE, uri, hostname);
    req_len = strlen(req);

    sysrc = s2n_send(tls_conn, req, req_len, &blocked);
    if( sysrc == -1 ) ERR_EXIT( ERR_WRITE_FAIL, "Call to s2n_send() failed" )
    else if( sysrc != req_len ) ERR_EXIT( ERR_WRITE_FAIL, "Got a partial write..." )
    
    for( done = 0; !done; )
    {
        sysrc = s2n_recv(tls_conn, buff, buff_len, &blocked);
/* printf("dbg:: s2n_recv: rc=%d\n", sysrc); */
        if( sysrc == 0 ) done = 1;

        else if( blocked == S2N_BLOCKED_ON_READ )
        {
            netbox.fd = sock;
            netbox.events = POLL_EVENTS_READ;
            netbox.revents = 0;

            sysrc = poll(&netbox, 1, MAX_IO_WAIT);
            if( !sysrc ) ERR_EXIT( ERR_READ_FAIL, "Timed out waiting to receive data" )
            if( sysrc == -1 ) ERR_EXIT( ERR_READ_FAIL, "Can't read from socket" )
	}

        else if( sysrc == -1 ) ERR_EXIT( ERR_READ_FAIL, "Can't read response from server" )

        else
        {
#ifdef SHOW_DEBUG
            printf("dbg:: Got %d bytes from server\n", sysrc);
            fflush(stdout);
#endif
/*            write( fileno(stdout), buff, sysrc); */
	}
    }

    return(tls_conn);
}
