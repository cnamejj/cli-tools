#ifndef ___CNAMEjj_SEND_UDP_MSG_H__

#define ___CNAMEjj_SEND_UDP_MSG_H__

/* --- */

#define OP_DEST_BOTH 1
#define OP_MESSAGE 2
#define OP_PORT 3
#define OP_HOST 4
#define OP_IPV4 5
#define OP_IPV6 6
#define OP_DEBUG 7

#define FL_DEST_BOTH "d"
#define FL_DEST_BOTH_2 "dest"
#define FL_DEST_BOTH_3 "destination"
#define FL_MESSAGE "m"
#define FL_MESSAGE_2 "msg"
#define FL_MESSAGE_3 "message"
#define FL_PORT "p"
#define FL_PORT_2 "port"
#define FL_HOST "h"
#define FL_HOST_2 "host"
#define FL_IPV4 "ipv4"
#define FL_IPV4_2 "4"
#define FL_IPV6 "ipv6"
#define FL_IPV6_2 "6"
#define FL_DEBUG "debug"

#define DEF_DEST_BOTH "localhost:48112"
#define DEF_MESSAGE "N/A"
#define DEF_PORT "48112"
#define DEF_HOST "localhost"
#define DEF_IPV4 "1"
#define DEF_IPV6 "1"
#define DEF_DEBUG "0"

#define ERRMSG_BAD_DEST "Destination '%s' invalid, expected 'host:port'."
#define ERRMSG_GETHOST_FAILED "Hostname '%s' could not be resolved to an acceptable IP address."
#define ERRMSG_INET_NTOP "Call to inet_ntop() failed with rc=%d."
#define ERRMSG_SOCKET_CALL "A socket() system call failed with rc=%d."
#define ERRMSG_SETSOCKOPT_CALL "Call to setsockopt() failed with rc=%d"
#define ERRMSG_SENDTO_FAIL "Call to sendto() failed with rc=%d"
#define ERRMSG_SENDTO_PARTIAL "Sent only %d of %d bytes requested."
#define ERRMSG_INET_PTON "Interal error, invalid address family passed to inet_pton() routing, host '%s'."

#define IPV6_LOOPBACK_ADDRESS "::1"

#define DO_IPV4 0x1
#define DO_IPV6 0x2

#define INT_ERR_DISPLAY_LEN 11
#define IP_DISPLAY_SIZE INET6_ADDRSTRLEN

/* -- Can't use this constants from "in6.h", so hardcode tem
#define SCOPE_LOOP __IPV6_ADDR_SCOPE_INTFACELOCAL
#define SCOPE_LINK __IPV6_ADDR_SCOPE_LINKLOCAL
#define SCOPE_SITE __IPV6_ADDR_SCOPE_SITELOCAL
#define SCOPE_GLOBAL __IPV6_ADDR_SCOPE_GLOBAL
 */
#define SCOPE_LOOP 0x01
#define SCOPE_LINK 0x02
#define SCOPE_SITE 0x05
#define SCOPE_GLOBAL 0x0e

#define DEBUG_LOW 1
#define DEBUG_MEDIUM 2
#define DEBUG_HIGH 3
#define DEBUG_NOISY 4

/* --- */

struct task_details {
    unsigned int use_ip, dest_port, found_family, debug;
    char *dest_host, *message, *err_msg;
    struct sockaddr_in dest4;
    struct sockaddr_in6 dest6;
};

/* --- */

#endif
