#ifndef ___CNAMEjj_HTTP_FETCH_H__

#define ___CNAMEjj_HTTP_FETCH_H__

/* --- */

#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

/* --- */

#ifdef USE_CLOCK_GETTIME
#  include <time.h>
#  define FRAC_RESOLUTION 0.000000001
#else
#  include <sys/time.h>
#  define FRAC_RESOLUTION 0.000001
#endif

/* --- */

#define FL_HEADER         "h"
#define FL_HEADER_2       "header"
#define FL_OUTPUT         "pe"
#define FL_OUTPUT_2       "output"
#define FL_URI            "uri"
#define FL_CONNHOST       "connhost"
#define FL_WEBSERVER      "webhost"
#define FL_URL            "url"
#define FL_PORT           "port"
#define FL_TIMERS         "time"
#define FL_TIMERS_2       "timers"
#define FL_DATA           "data"
#define FL_LOOP           "loop"
#define FL_HTML           "html"
#define FL_NUMBER         "num"
#define FL_PAUSE          "wait"
#define FL_XHEADER        "hfield"
#define FL_USESTDERR      "stderr"
#define FL_AUTH           "auth"
#define FL_TIMERHEADERS   "timeheaders"
#define FL_TIMERHEADERS_2 "timeh"
#define FL_TIMEOUT        "timeout"
#define FL_PROXY          "proxy"
#define FL_PACKETIME      "packetime"
#define FL_CLIENTIP       "clip"
#define FL_PTHRU          "connthru"
#define FL_DEBUG          "debug"

#define OP_HEADER         1
#define OP_OUTPUT         2
#define OP_URI            3
#define OP_CONNHOST       4
#define OP_WEBSERVER      5
#define OP_URL            6
#define OP_PORT           7
#define OP_TIMERS         8
#define OP_DATA           9
#define OP_LOOP           10
#define OP_HTML           11
#define OP_NUMBER         12
#define OP_PAUSE          13
#define OP_XHEADER        14
#define OP_USESTDERR      15
#define OP_AUTH           16
#define OP_TIMERHEADERS   17
#define OP_TIMEOUT        18
#define OP_PROXY          19
#define OP_PACKETIME      20
#define OP_CLIENTIP       21
#define OP_PTHRU          22
#define OP_DEBUG          23

#define DEF_HEADER         "0"
#define DEF_HEADER_2       DEF_HEADER
#define DEF_OUTPUT         "1"
#define DEF_OUTPUT_2       DEF_OUTPUT
#define DEF_URI            ""
#define DEF_CONNHOST       ""
#define DEF_WEBSERVER      ""
#define DEF_URL            "http://www.cnn.com"
#define DEF_PORT           "-1"
#define DEF_TIMERS         "0"
#define DEF_TIMERS_2       DEF_TIMERS
#define DEF_DATA           "0"
#define DEF_LOOP           "0"
#define DEF_HTML           "0"
#define DEF_NUMBER         "0"
#define DEF_PAUSE          "0"
#define DEF_XHEADER        ""
#define DEF_USESTDERR      "0"
#define DEF_AUTH           ""
#define DEF_TIMERHEADERS   "1"
#define DEF_TIMERHEADERS_2 DEF_TIMEHEADERS
#define DEF_TIMEOUT        "300"
#define DEF_PROXY          ""
#define DEF_PACKETIME      "0"
#define DEF_CLIENTIP       ""
#define DEF_PTHRU          "0"
#define DEF_DEBUG          "0"

#define DEBUG_NONE 0
#define DEBUG_LOW1 1
#define DEBUG_LOW2 2
#define DEBUG_LOW3 3
#define DEBUG_MEDIUM1 4
#define DEBUG_MEDIUM2 5
#define DEBUG_MEDIUM3 6
#define DEBUG_HIGH1 7
#define DEBUG_HIGH2 8
#define DEBUG_HIGH3 9
#define DEBUG_NOISY1 10
#define DEBUG_NOISY2 11
#define DEBUG_NOISY3 12

/* Default poll() timeout in milliseconds */
#define DEF_WAIT_TIMEOUT 300000

#define HTTP_EOL "\r\n"

#define NO_PORT -1
#define NO_SOCK -1

#define UNDEFINED_ERROR "A unspecified error halted execution of the program."

#define HTML_RESP_HEADER "<HEAD><pre>\n"

#define HTTP_HEADER_XPREF "X-"

#define DEFAULT_FETCH_USER_AGENT "Mozilla/5.0 Gecko/20100101 Firefox/23.0"

#define DEFAULT_FETCH_PORT 80

#define ROOT_URI "/"
#define INVALID_IP "Invalid-IP"

#define IP_UNKNOWN 0
#define IP_V4      1
#define IP_V6      2

/* #define READ_BUFF_SIZE 65536 */
#define READ_BUFF_SIZE 262144

#ifndef SCOPE_LINK
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
#endif

#define POLL_EVENTS_READ POLLIN | POLLRDNORM | POLLPRI
#define POLL_EVENTS_WRITE POLLOUT | POLLWRNORM
#define POLL_EVENTS_ANY POLL_EVENTS_READ | POLL_EVENTS_WRITE | POLLERR | POLLHUP | POLLNVAL | POLLRDBAND | POLLWRBAND
#define POLL_EVENTS_ERROR POLLERR | POLLHUP | POLLNVAL

#define LS_START           0x0000
#define LS_PARSED_OPTIONS  0x0001
#define LS_FIND_CONNECTION 0x0002
#define LS_GEN_REQUEST     0x0004
#define LS_CONNECT_LOOKUP  0x0008
#define LS_ESTAB_CONNECT   0x0010
#define LS_SENT_REQUEST    0x0020
#define LS_READ_READY      0x0040
#define LS_GOT_RESPONSE    0x0080
#define LS_USE_GAI_ERRNO   0x0100
#define LS_CKPT_SETUP      0x0200
#define LS_NO_CONNECTION   0x0400
#define LS_NO_REQUEST      0x0800
#define LS_NO_RESPONSE     0x1000
#define LS_NO_PAYLOAD      0x2000
#define LS_NO_DNS_CONNECT  0x4000

#define EVENT_BLANK          0
#define EVENT_START_FETCH    1
#define EVENT_DNS_LOOKUP     2
#define EVENT_CONNECT_SERVER 3
#define EVENT_REQUEST_SENT   4
#define EVENT_FIRST_RESPONSE 5
#define EVENT_READ_PACKET    6
#define EVENT_READ_ALL_DATA  7

/* --- */

#define FINAL_FETCH_REQUEST_TEMPLATE "\
GET %uri% HTTP/1.1\r\n\
%hostcomment%Host: %webhost%\r\n\
User-Agent: %agent%\r\n\
Accept: */*\r\n\
Connection: close\r\n\
DNT: 1\r\n\
\r\n"

#define FETCH_REQUEST_TEMPLATE "\
GET %uri% HTTP/1.1\n\
%hostcomment%Host: %webhost%\n\
User-Agent: %agent%\n\
Accept: */*\n\
Connection: close\n\
DNT: 1\n\
\n"

#define PATT_URI "%uri%"
#define PATT_HOST_COMMENT "%hostcomment%"
#define PATT_HOST_NAME "%webhost%"
#define PATT_USER_AGENT "%agent%"

/* --- */

#define SHOW_OPT_IF_DEBUG( NAME) \
if( out->debug_level >= DEBUG_HIGH1 && (co->flags & OP_FL_FOUND)) \
  fprintf( out->info_out, "Opt #%d, %s '%s'\n", co->opt_num, NAME, co->val);

#define SPSP( ST) ST ? ST : "\0"

/* --- */

struct summary_stats {
  float start_time, lookup_time, connect_time, request_time, response_time,
    complete_time;
};

struct data_block {
  int len;
  char *data;
};

struct ckpt_entry {
  long sec, frac;
};

struct ckpt_chain {
  int event;
  struct ckpt_entry clock;
  struct data_block *detail;
  struct ckpt_chain *next;
};

struct chain_position {
  char *position;
  struct ckpt_chain *chain;
};

struct target_info {
  char *http_host, *conn_host, *conn_uri, *conn_url;
  char *ipv4, *ipv6;
  char *auth_user, *auth_passwd;
  char *proxy_url, *proxy_host, *proxy_ipv4, *proxy_ipv6;
  int conn_port, proxy_port, conn_pthru;
  struct value_chain *extra_headers;
};

struct output_options {
  int out_html, debug_level;
  FILE *info_out, *err_out;
};

struct display_settings {
  int show_head, show_data, show_timers, show_timerheaders,
    show_packetime, show_help, show_complete, show_number;
};

struct exec_controls {
  int loop_count, loop_pause, conn_timeout;
  char *client_ip;
};

struct fetch_status {
  long response_len;
  int ip_type, conn_sock, request_len, wait_timeout, last_state,
    end_errno;
  char *err_msg, *request;
  float clock_res;
  struct sockaddr_in sock4;
  struct sockaddr_in6 sock6;
  struct ckpt_chain *checkpoint, *lastcheck;
};

struct plan_data {
  struct target_info *target;
  struct display_settings *disp;
  struct exec_controls *run;
  struct output_options *out;
  struct fetch_status *status;
};

/* --- */

#define MSG_SHOW_SYNTAX "\n\
Syntax is: %s <options>\n\
Options are:\n\
\n\
  <--header> | <--h>\n\
  <--output> | <--pe>\n\
  <--uri uri>\n\
  <--connhost hostname\n\
  <--webhost hostname\n\
  <--url url>\n\
  <--port port>\n\
  <--timers> | <--time>\n\
  <--data>\n\
  <--loop number-of-fetches>\n\
  <--html>\n\
  <--num>\n\
  <--wait seconds>\n\
  <--hfield 'Field: value'\n\
  <--stderr>\n\
  <--auth user:password\n\
  <--timeheaders> | <--timeh>\n\
  <--timeout seconds>\n\
  <--proxy hostname:port>\n\
  <--packetime>\n\
  <--clip ip-address\n\
  <--connthru>\n\
  <--debug debug-level>\n\
"

/* --- */

struct plan_data *figure_out_plan( int *returncode, int narg, char **opts);

struct plan_data *allocate_plan_data();

int find_connection( struct plan_data *plan);

int execute_fetch_plan( struct plan_data *plan);

int construct_request( struct plan_data *plan);

void clear_counters( int *rc, struct plan_data *plan);

void lookup_connect_host( int *rc, struct plan_data *plan);

void connect_to_server( int *rc, struct plan_data *plan);

void send_request( int *rc, struct plan_data *plan);

void wait_for_reply( int *rc, struct plan_data *plan);

void pull_response( int *rc, struct plan_data *plan);

void display_output( int *rc, struct plan_data *plan);

int capture_checkpoint( struct fetch_status *anchor, int event_type);

void free_data_block( struct data_block *detail);

int add_data_block( struct ckpt_chain *checkpoint, int len, char *buff);

float calc_time_difference( struct ckpt_entry *start, struct ckpt_entry *end, float clock_res);

struct chain_position *find_header_break( struct ckpt_chain *chain);

/* --- */

#endif
