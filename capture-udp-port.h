#ifndef ___CNAMEjj_CAPTURE_UDP_PORT_H__

#define ___CNAMEjj_CAPTURE_UDP_PORT_H__

/* --- */

#define OP_SERVER 1
#define OP_USER 2
#define OP_PORT 3
#define OP_HOST 4
#define OP_IPV4 5
#define OP_IPV6 6
#define OP_LOGFILE 7
#define OP_MODE 8
#define OP_DEBUG 9
#define OP_GROUP 10
#define OP_TRUNC 11
#define OP_APPEND 12
#define OP_HELP 13

#define FL_SERVER "s"
#define FL_SERVER_2 "server"
#define FL_USER "u"
#define FL_USER_2 "user"
#define FL_PORT "p"
#define FL_PORT_2 "port"
#define FL_HOST "h"
#define FL_HOST_2 "host"
#define FL_IPV4 "ipv4"
#define FL_IPV4_2 "4"
#define FL_IPV6 "ipv6"
#define FL_IPV6_2 "6"
#define FL_LOGFILE "l"
#define FL_LOGFILE_2 "log"
#define FL_MODE "m"
#define FL_MODE_2 "mode"
#define FL_DEBUG "debug"
#define FL_GROUP "g"
#define FL_GROUP_2 "group"
#define FL_TRUNC "truncate"
#define FL_TRUNC_2 "t"
#define FL_APPEND "append"
#define FL_APPEND_2 "a"
#define FL_HELP "help"

#define DEF_SERVER "localhost:48112"
#define DEF_USER ""
#define DEF_PORT "48112"
#define DEF_HOST "localhost"
#define DEF_IPV4 "0"
#define DEF_IPV6 "0"
#define DEF_LOGFILE "udp-capture.log"
#define DEF_MODE "0644"
#define DEF_DEBUG "0"
#define DEF_GROUP ""
#define DEF_TRUNC "0"
#define DEF_APPEND "1"
#define DEF_HELP "0"

#define NO_SWITCH_NEEDED "run-asis"

#define BUFFER_SIZE 65535
#define LOG_BUFFER_SIZE (BUFFER_SIZE + 1) * 2

#define SUBSET_READ  0x4
#define SUBSET_WRITE 0x2
#define SUBSET_EXEC  0x1

#define UNKNOWN_IP "Unknown-IP"

#define IPV6_DISPLAY_SIZE INET6_ADDRSTRLEN

#define TIME_DISPLAY_SIZE 256
#define TIME_DISPLAY_FORMAT "%y/%m/%d-%H:%M:%S "

#define LENGTH_DISPLAY_SIZE 32
#define LENGTH_DISPLAY_FORMAT " %d "

/* --- */

#define MSG_SHOW_SYNTAX "\n\
Syntax is: %s <options>\n\
Options are:\n\
  <--server hostname:port> | <-s hostname:port>\n\
  <--host hostname> | <-h hostname>\n\
  <--port port> | <-p port>\n\
  <--user name> | <-u name>\n\
  <--group name> | <-g name>\n\
  <--log logfile> | <-l logfile>\n\
  <--mode mode> | <-m mode> \n\
  <--truncate> | <-t>\n\
  <--append> | <-a>\n\
  <--ipv4> | <-4>\n\
  <--ipv6> | <-6>\n\
  <--help>\n\
  <--debug level>\n\
"

/* --- */

#endif

