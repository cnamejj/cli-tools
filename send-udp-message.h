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
#define OP_HELP 8

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
#define FL_HELP "help"

#define DEF_DEST_BOTH "localhost:48112"
#define DEF_MESSAGE "N/A"
#define DEF_PORT "48112"
#define DEF_HOST "localhost"
#define DEF_IPV4 "0"
#define DEF_IPV6 "0"
#define DEF_DEBUG "0"
#define DEF_HELP "0"

/* --- */

#define MSG_SHOW_SYNTAX "\n\
Syntax is: %s <options>\n\
Options are:\n\
  <--message msg-text> | <--msg msg-text> | <-m msg-text>\n\
  <--destination hostname:port> | <--dest hostname:port> | <-d hostname:port>\n\
  <--host hostname> | <-h hostname>\n\
  <--port port> | <-p port>\n\
  <--ipv4> | <-4>\n\
  <--ipv6> | <-6>\n\
  <--help>\n\
  <--debug level>\n\
\n\
The 'hostname' can be a domain name or an IP address.\n\
"

/* --- */

#endif

