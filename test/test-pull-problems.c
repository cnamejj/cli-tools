#include "../err_ref.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/time.h>
#include <unistd.h>

/* --- */

#define DEF_URI "/"
#define HTTP_PORT 80
/*
#define POLL_EVENTS_WRITE POLLOUT | POLLWRNORM
#define POLL_EVENTS_READ POLLIN | POLLRDNORM | POLLPRI
 */
#define POLL_EVENTS_WRITE POLLOUT | POLLWRNORM
#define POLL_EVENTS_READ POLLIN | POLLRDNORM
#define BUFFER_SIZE 262144

#define FETCH_REQUEST_TEMPLATE "\
GET %s HTTP/1.1\r\n\
Host: %s\r\n\
User-Agent: Mozilla/5.0 Gecko/20100101 Firefox/23.0\r\n\
Accept: */*\r\n\
Connection: close\r\n\
DNT: 1\r\n\
\r\n"

/* --- */

#define LEAVE( MSG) \
{ \
    fprintf( stderr, "Err: %s\n", MSG); \
    exit( 1); \
}

#define LEAVE_E1( MSG, CODE) \
{ \
    fprintf( stderr, "Err: "); \
    fprintf( stderr, MSG, CODE); \
    fprintf( stderr, "\n"); \
    exit( 1); \
}

#define LEAVE_SYSCALL( CALL, CODE) \
{ \
    fprintf( stderr, "Err: Got error rc(%d) from %s() call.\n", CODE, CALL); \
    exit( 1); \
}

/* --- */

void debug_timelog( char *tag);

void debug_timelog( char *tag)

{
    static int seq = 0;
    long diff_sec, diff_sub, now_sec, now_sub, top = 1000000;
    static struct timeval now, prev = { 0, 0 };

    seq++;
    (void) gettimeofday( &now, 0);

    diff_sec = now.tv_sec - prev.tv_sec;
    diff_sub = now.tv_usec - prev.tv_usec;

    now_sec = now.tv_sec;
    now_sub = now.tv_usec;

    if( diff_sub < 0)
    {
        diff_sec--;
        diff_sub += top;
    }

    fprintf( stdout, "dbg:: %3d. %11ld.%06ld %11ld.%06ld (%s)\n", seq, now_sec, now_sub, diff_sec, diff_sub, tag);

    prev.tv_sec = now.tv_sec;
    prev.tv_usec = now.tv_usec;

    return;
}

/* --- */

int main( int narg, char **opts)

{
    int rc = RC_NORMAL, sysrc = 0, ip_type = 0, salen, sock, wait_timeout = 30000, request_len, seq = 0, done, buff_len = BUFFER_SIZE;
    char *host = 0, *uri = 0, *st, *strc, *request, *last;
    char display_ip[ INET6_ADDRSTRLEN + 1], buff[ BUFFER_SIZE];
    struct addrinfo hints, *hostrecs = 0;
    struct sockaddr_in6 sock6;
    struct sockaddr_in sock4;
    struct sockaddr *sa = 0;
    struct linger linger;
    struct pollfd netbox;

    debug_timelog( "Start");
    memset( &sock4, '\0', sizeof sock4);
    memset( &sock6, '\0', sizeof sock6);

    if( narg < 2) LEAVE( "No hostname given.")
    host = opts[ 1];

    if( narg < 3) uri = DEF_URI;
    else uri = opts[ 2];

    /* ---
     * -  DNS lookup
     */
    
    debug_timelog( "Lookup");

    hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = 0;
    hints.ai_canonname = 0;
    hints.ai_next = 0;

    sysrc = getaddrinfo( host, 0, &hints, &hostrecs);
    if( sysrc == EAI_NONAME) LEAVE( "Got EAI_NONAME from getaddrinfo() call")
    if( sysrc) LEAVE_SYSCALL( "gettaddrinfo", errno)

    if( hostrecs->ai_family == AF_INET6)
    {
        ip_type = AF_INET6;
        memcpy( &sock6, hostrecs->ai_addr, sizeof sock6);
        sock6.sin6_port = htons( HTTP_PORT);

        st = display_ip;
        strc = (char *) inet_ntop( AF_INET6, &sock6.sin6_addr, st, (sizeof display_ip));
        if( !strc) LEAVE( "Bogus IPV6 address in hostrec")

        sa = (struct sockaddr *) &sock6;
        salen = sizeof sock6;
    }
    else if( hostrecs->ai_family == AF_INET)
    {
        ip_type = AF_INET;
        memcpy( &sock4, hostrecs->ai_addr, sizeof sock4);
        sock4.sin_port = htons( HTTP_PORT);

        st = display_ip;
        strc = (char *) inet_ntop( AF_INET, &sock4.sin_addr, st, (sizeof display_ip));
        if( !strc) LEAVE( "Bogus IPV4 address in hostrec")

        sa = (struct sockaddr *) &sock4;
        salen = sizeof sock4;
    }
    else LEAVE_E1( "WTF? DNS records isn't IPV4 or IPV6, got family(%d)", hostrecs->ai_family)

    freeaddrinfo( hostrecs);

    /* ---
     * - connect to the webserver
     */

    debug_timelog( "Connect");

    sock = socket( ip_type, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if( sock == -1) LEAVE_SYSCALL( "socket", errno)

    linger.l_onoff = 1;
    linger.l_linger = 300;
    sysrc = setsockopt( sock, SOL_SOCKET, SO_LINGER, &linger, sizeof linger);
    if( sysrc < 0) LEAVE_SYSCALL( "setsockopt", errno)

    sysrc = connect( sock, sa, salen);
    if( sysrc != 0 && errno != EINPROGRESS) LEAVE_SYSCALL( "connect", errno)

    netbox.fd = sock;
    netbox.events = POLL_EVENTS_WRITE;
    netbox.revents = 0;

    debug_timelog( "ConnWait");
    sysrc = poll( &netbox, 1, wait_timeout);
    if( sysrc == 0) LEAVE( "Call to poll() timed out")
    if( sysrc == -1) LEAVE_SYSCALL( "poll", errno)

    /* ---
     * - send the HTTP request
     */

    debug_timelog( "Request");

    request_len = strlen( FETCH_REQUEST_TEMPLATE + 1) + strlen( host) + strlen( uri);
    request = (char *) malloc( request_len);
    if( !request) LEAVE( "Can't allocate memory for HTTP request")
    snprintf( request, request_len, FETCH_REQUEST_TEMPLATE, uri, host);

    sysrc = write( sock, request, request_len);
    if( sysrc != request_len) LEAVE_SYSCALL( "write", errno);

    /* ---
     * - wait for response
     */

    debug_timelog( "ResponseWait");

    netbox.fd = sock;
    netbox.events = POLL_EVENTS_READ;
    netbox.revents = 0;

    sysrc = poll( &netbox, 1, wait_timeout);
    if( sysrc == 0) LEAVE( "Call to poll() timed out")
    if( sysrc == -1) LEAVE_SYSCALL( "poll", errno)

    /* ---
     * - read the payload
     */

    debug_timelog( "Reading");

    for( done = 0; !done; )
    {
        sysrc = read( sock, buff, buff_len);
        if( sysrc == -1 && errno == EINTR) printf( "Ignoring 'EINTR' return code\n");
        else if( sysrc == -1) LEAVE_SYSCALL( "read", errno)
        else if( !sysrc) done = 1;
        else
        {
            printf( "%d. Read %d bytes\n", ++seq, sysrc);
            for( last = buff + sysrc, st = buff; st < last; st++) fputc( *st, stderr);
	}

        if( !done)
        {
            netbox.fd = sock;
            netbox.events = POLL_EVENTS_READ;
            netbox.revents = 0;

            sysrc = poll( &netbox, 1, wait_timeout);
            if( sysrc == 0) LEAVE( "Call to poll() timed out")
            if( sysrc == -1) LEAVE_SYSCALL( "poll", errno)
	}
    }
        
    close( sock);

    /* --- */

    printf( "\n\n- - - Summary - - -\n");
    printf( "Fetch specs: http://%s%s\n", host, uri);

    st = display_ip;
    printf( "IPv");
    if( ip_type == AF_INET6) printf( "6");
    else printf( "4");
    printf( " address: %s\n", st);

    printf( "HTTP request: ");
    for( st = request; *st; st++)
    {
        if( *st == '\r') fputc( '!', stdout);
        else if( *st == '\n')
        {
            fputc( *st, stdout);
            printf( "              ");
        }
        else fputc( *st, stdout);
    }

    printf( "\n");

    /* --- */

    return( rc);
}
