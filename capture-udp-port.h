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

#define NO_SWITCH_NEEDED "run-asis"

#define BUFFER_SIZE 1500

#define SUBSET_READ  0x4
#define SUBSET_WRITE 0x2
#define SUBSET_EXEC  0x1

#define LOG_OPEN_FLAGS O_WRONLY | O_APPEND | O_APPEND

/* --- */

#endif
