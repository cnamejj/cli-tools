#ifndef ___CNAMEjj_NET_TASK_DATA_H__

#define ___CNAMEjj_NET_TASK_DATA_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* --- */

struct task_details *allocate_plan_data();

int get_destination_ip( struct task_details *plan);

int parse_destination_value( struct task_details *plan, char *destination);

/* --- */

#define IPV6_LOOPBACK_ADDRESS "::1"

#define DO_IPV4 0x1
#define DO_IPV6 0x2

#define INT_ERR_DISPLAY_LEN 11
#define IP_DISPLAY_SIZE INET6_ADDRSTRLEN

#define DEBUG_LOW 1
#define DEBUG_MEDIUM 2
#define DEBUG_HIGH 3
#define DEBUG_NOISY 4

#define LOG_OPEN_FLAGS O_WRONLY | O_CREAT

/* --- */

struct task_details {
    unsigned int use_ip, target_port, found_family, logmode, debug, openflags,
      show_help, msg_in_hex;
    char *target_host, *message, *logfile, *runuser, *rungroup, *err_msg;
    struct sockaddr_in dest4;
    struct sockaddr_in6 dest6;
};

/* --- */

#endif
