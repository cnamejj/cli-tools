#ifndef ___CNAMEjj_NET_TASK_DATA_H__

#define ___CNAMEjj_NET_TASK_DATA_H__

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
    unsigned int use_ip, target_port, found_family, logmode, debug, show_help;
    char *target_host, *message, *logfile, *runuser, *rungroup, *err_msg;
    struct sockaddr_in dest4;
    struct sockaddr_in6 dest6;
};

/* --- */

#endif
