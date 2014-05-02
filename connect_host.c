#include <string.h>
#include <errno.h>
#include <poll.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifndef linux
#include <fcntl.h>
#endif
#include <unistd.h>

#include "cli-sub.h"
#include "err_ref.h"

/* --- */

int connect_host(int *retcode, char *hostname, int port, int timeout, int protocol)

{
    int sock = NO_SOCK, sock_type, sock_port, salen, rc = RC_NORMAL, sysrc;
    struct sockaddr_in sock4, *hrec4;
    struct sockaddr_in6 sock6, *hrec6;
    struct linger linger;
    struct pollfd pset;
    struct sockaddr *sa = 0;
    struct addrinfo hints, *hostrecs = 0;

    /* --- */

    memset(&sock4, EOS_CH, sizeof sock4);
    memset(&sock6, EOS_CH, sizeof sock6);

    sock4.sin_family = AF_INET;
    sock4.sin_port = 0;
    sock4.sin_addr.s_addr = 0;

    sock6.sin6_family = AF_INET6;
    sock6.sin6_port = 0;
    sock6.sin6_flowinfo = 0;
    sock6.sin6_addr = in6addr_loopback;
    sock6.sin6_scope_id = SCOPE_LINK;

    /* --- */

    if(is_ipv6_address(hostname))
    {
        sock_type = AF_INET6;
        salen = sizeof sock6;
        sysrc = inet_pton(AF_INET6, hostname, &sock6.sin6_addr);
        if( sysrc != 1) rc = ERR_INVALID_DATA;
    }
    else if(is_ipv4_address(hostname))
    {
        sock_type = AF_INET;
        salen = sizeof sock4;
        sysrc = inet_pton(AF_INET, hostname, &sock4.sin_addr);
        if( sysrc != 1) rc = ERR_INVALID_DATA;
    }
    else
    {
        if(protocol == AF_INET6)
        {
            sock_type = AF_INET6;
            salen = sizeof sock6;
        }
        else
        {
            sock_type = AF_INET;
            salen = sizeof sock4;
        }

        hints.ai_flags = 0;
        hints.ai_family = sock_type;
        hints.ai_socktype = 0;
        hints.ai_protocol = 0;
        hints.ai_addrlen = 0;
        hints.ai_addr = 0;
        hints.ai_canonname = 0;
        hints.ai_next = 0;

        sysrc = getaddrinfo(hostname, 0, &hints, &hostrecs);
        if(sysrc == EAI_NONAME) rc = ERR_GETHOST_FAILED;
        else if(sysrc) rc = ERR_SYS_CALL;
        else if(sock_type == AF_INET6)
        {
            hrec6 = (struct sockaddr_in6 *) hostrecs->ai_addr;
            sock6.sin6_addr = hrec6->sin6_addr;
            sock6.sin6_scope_id = hrec6->sin6_scope_id;
	}
        else
        {
            hrec4 = (struct sockaddr_in *) hostrecs->ai_addr;
            sock4.sin_addr = hrec4->sin_addr;
	}

        freeaddrinfo(hostrecs);
    }

    /* --- */

    if(rc == RC_NORMAL)
    {
        sock = socket(sock_type, SOCKET_OPTS, 0);
        if(sock == -1) rc = ERR_SOCKET_FAILED;
    }

    if(rc == RC_NORMAL)
    {
        linger.l_onoff = 1;
        linger.l_linger = 300;
        sysrc = setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger, sizeof linger);
        if( sysrc < 0) rc = ERR_SYS_CALL;
    }

#ifndef linux
    if(rc == RC_NORMAL)
    {
        int flags;

        flags = fcntl(sock, F_GETFL);
        if(flags >= 0) sysrc = fcntl( sock, F_SETFL, flags | FNONBLOCK);
        if(flags < 0) rc = ERR_FCNTL_FAILED;
    }
#endif

    if(rc == RC_NORMAL)
    {
        if(port == NO_PORT) sock_port = htons(DEF_CLIENT_PORT);
        else sock_port = htons(port);

        if(sock_type == AF_INET6)
        {
            sock6.sin6_port = sock_port;
            sa = (struct sockaddr *) &sock6;
	}
        else
        {
            sock4.sin_port = sock_port;
            sa = (struct sockaddr *) &sock4;
	}

        sysrc = connect(sock, sa, salen);
        if(errno == EINPROGRESS)
        {
            pset.fd = sock;
            pset.events = POLL_EVENTS_WRITE;
            pset.revents = 0;

            errno = 0;
            sysrc = poll(&pset, 1, timeout);
            if(sysrc == 0) rc = ERR_POLL_TIMEOUT;
            else if(sysrc == -1) rc = ERR_SYS_CALL;
	}
        else if(sysrc != 0) rc = ERR_CONN_FAILED;
    }

    /* --- */

    *retcode = rc;
    if(*retcode != RC_NORMAL) sock = NO_SOCK;

    return(sock);
}
