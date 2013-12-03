#ifndef ___CNAMEjj_HTTP_FETCH_H__

#define ___CNAMEjj_HTTP_FETCH_H__

/* --- */

#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

/* --- */

#ifdef USE_CLOCK_GETTIME
#  include <time.h>
#  define FRAC_RESOLUTION 0.000000001
#else
#  define FRAC_RESOLUTION 0.000001
#endif

#pragma GCC diagnostic ignored "-Woverlength-strings"

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
#define FL_TCP4           "tcp4"
#define FL_TCP4_2         "4"
#define FL_TCP6           "tcp6"
#define FL_TCP6_2         "6"
#define FL_HTTP10         "http1.0"
#define FL_HTTP11         "http1.1"
#define FL_INTERFACE      "interface"
#define FL_INTERFACE_2    "if"

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
#define OP_TCP4           24
#define OP_TCP6           25
#define OP_HTTP10         26
#define OP_HTTP11         27
#define OP_INTERFACE      28

#define DEF_HEADER         "1"
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
#define DEF_DATA           "1"
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
#define DEF_TCP4           "0"
#define DEF_TCP4_2         "0"
#define DEF_TCP6           "0"
#define DEF_TCP6_2         "0"
#define DEF_HTTP10         "0"
#define DEF_HTTP11         "1"
#define DEF_INTERFACE      ""

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

#define NO_CR_HTTP_EOL "\r\n"

#define NO_PORT -1
#define NO_SOCK -1

#define UNDEFINED_ERROR "A unspecified error halted execution of the program."

#define HTML_RESP_HEADER "\
Content-type: text/html\r\n\
\r\n\
<HEAD>\n\
"

#define HTML_RESP_END "\
</body>\n\
</html>\n\
"

/* #define HTML_RESP_IFRAME_START "<iframe seamless sandbox srcdoc=\"" */

#define HTML_HEIGHT_HEAD ""
#define HTML_HEIGHT_DATA " height=\"100%\""

#define HTML_RESP_IFRAME_START "<iframe seamless%s width=\"100%%\" sandbox srcdoc=\""
#define HTML_RESP_IFRAME_END "\"></iframe>\n"

#define HTML_PREFORMAT_START "<pre>"
#define HTML_PREFORMAT_END "</pre>"
#define HTML_FIXEDLINE_START "<br><tt>"
#define HTML_FIXEDLINE_END "</tt>"
#define HTML_BREAK "<br>"
#define HTML_BREAK_NOOP "<!-- <br> -->"
#define HTML_GT_ESCAPE "&gt;"
#define HTML_LT_ESCAPE "&lt;"
#define HTML_DQ_ESCAPE "&quot;"
#define HTML_AM_ESCAPE "&amp;"

#define EMPTY_STRING ""
#define EOL_STRING "\n"
#define SPACE_STRING " "
#define AMPER_STRING "&"
#define DQUOTE_STRING "\""

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
  /* -- Can't use these constants from "in6.h", so hardcode them
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
#define LS_HTML_FORM_SENT  0x8000

#define EVENT_BLANK          0
#define EVENT_START_FETCH    1
#define EVENT_DNS_LOOKUP     2
#define EVENT_CONNECT_SERVER 3
#define EVENT_REQUEST_SENT   4
#define EVENT_FIRST_RESPONSE 5
#define EVENT_READ_PACKET    6
#define EVENT_READ_ALL_DATA  7

/* --- */

#define FETCH_REQUEST_TEMPLATE "\
GET %uri% %httpprot%\r\n\
%hostcomment%Host: %webhost%\r\n\
User-Agent: %agent%\r\n\
Accept: */*\r\n\
Connection: close\r\n\
DNT: 1\r\n\
%exheaders%\
\r\n"

#define NO_CR_FETCH_REQUEST_TEMPLATE "\
GET %uri% %httpprot%\n\
%hostcomment%Host: %webhost%\n\
User-Agent: %agent%\n\
Accept: */*\n\
Connection: close\n\
DNT: 1\n\
%exheaders%\
\n"

/* --- */

#define TIME_SUMMARY_HEADER_1 "\
-Date--Time-     -------- Elapsed Time ------- Total -------- Transfer --------- -- Received Packet Size --- ---- Inter-Packet Lag ----- -- Per-Packet Xfer Rate ---\
"
#define TIME_SUMMARY_HEADER_2 "\
YrMnDyHrMnSe HRC   DNS  Conn  Send 1stRD Close  Time AllByt PayByt Tot#/S Dat#/S   StDev  Skewness  Kurtosis   StDev  Skewness  Kurtosis   StDev  Skewness  Kurtosis\
"
#define TIME_SUMMARY_HEADER_3 "\
------------ --- ----- ----- ----- ----- ----- ----- ------ ------ ------ ------ ------- --------- --------- ------- --------- --------- ------- --------- ---------\
"

#define TIME_SUMMARY_HEADER_SEQ_1 "     "
#define TIME_SUMMARY_HEADER_SEQ_2 " Seq "
#define TIME_SUMMARY_HEADER_SEQ_3 "---- "

#define TIME_DISPLAY_FORMAT "%y%m%d%H%M%S"
#define TIME_DISPLAY_SIZE 15

#define HTTP_HEAD_TRANSFER_ENCODING "Transfer-Encoding"
#define HTTP_HEAD_CONTENT_TYPE "Content-Type"

#define ENC_TYPE_CHUNKED "chunked"

#define HTTP_FORM_TEMPLATE "\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\
<html><head><title>Http-Fetch tester form</title>\n\
</head>\n\
<body bgcolor=\"#ffffff\">\n\
\n\
<form action=\"http://%s%s%s%s\" method=\"GET\">\n\
\n\
<!-- These options aren't useful in a CGI: stderr -->\n\
<!-- Options that are't implemented yet: html, auth, proxy, connthru -->\n\
\n\
<table>\n\
  <tr><td colspan=2>What to fetch:</td></tr>\n\
  <tr><td>Full URL:</td><td><input type=\"text\" name=\"url\"></td></tr>\n\
  <tr><td>Connect to host/ip:</td><td><input type=\"text\" name=\"connhost\"></td></tr>\n\
  <tr><td>Name of webserver:</td><td><input type=\"text\" name=\"webhost\"></td></tr>\n\
  <tr><td>Connect to this port:</td><td><input type=\"text\" name=\"port\" value=\"80\"></td></tr>\n\
  <tr><td>URI to request:</td><td><input type=\"text\" name=\"uri\"></td></tr>\n\
  <tr><td>Extra HTTP header(s):</td><td><input type=\"text\" name=\"hfield\"></td></tr>\n\
  <tr><td>Extra HTTP header(s):</td><td><input type=\"text\" name=\"hfield\"></td></tr>\n\
  <tr><td>Extra HTTP header(s):</td><td><input type=\"text\" name=\"hfield\"></td></tr>\n\
\n\
  <tr><td>&nbsp;</td><td>&nbsp;</td></tr>\n\
  <tr><td colspan=2>What to display:</td></tr>\n\
<!--  <tr><td>Show complete results:</td><td> Yes<input type=\"radio\" value=\"yes\" name=\"output\">\n\
No<input type=\"radio\" value=\"no\" name=\"output\" checked></td></tr>\n\
-->  <tr><td>Show HTTP header:</td><td> Yes<input type=\"radio\" value=\"yes\" name=\"header\">\n\
No<input type=\"radio\" value=\"no\" name=\"header\" checked></td></tr>\n\
  <tr><td>Show HTTP data:</td><td> Yes<input type=\"radio\" value=\"yes\" name=\"data\">\n\
No<input type=\"radio\" value=\"no\" name=\"data\" checked></td></tr>\n\
  <tr><td>Show summary stats:</td><td> Yes<input type=\"radio\" value=\"yes\" name=\"timers\" checked>\n\
No<input type=\"radio\" value=\"no\" name=\"timers\"></td></tr>\n\
  <tr><td>Number summary stats:</td><td> Yes<input type=\"radio\" value=\"yes\" name=\"num\" checked>\n\
No<input type=\"radio\" value=\"no\" name=\"num\"></td></tr>\n\
  <tr><td>Show column headers:</td><td> Yes<input type=\"radio\" value=\"yes\" name=\"timeheaders\" checked>\n\
No<input type=\"radio\" value=\"no\" name=\"timeheaders\"></td></tr>\n\
  <tr><td>Show packet receipt times:</td><td> Yes<input type=\"radio\" value=\"yes\" name=\"packetime\">\n\
No<input type=\"radio\" value=\"no\" name=\"packetime\" checked></td></tr>\n\
\n\
  <tr><td>&nbsp;</td><td>&nbsp;</td></tr>\n\
  <tr><td colspan=2>Recipe:</td></tr>\n\
  <tr><td>Network timeout:</td><td><input type=\"text\" name=\"timeout\" value=\"30000\"></td></tr>\n\
  <tr><td>How many times to fetch:</td><td><input type=\"text\" name=\"loop\" value=\"1\"></td></tr>\n\
  <tr><td>Pause time between fetches:</td><td><input type=\"text\" name=\"wait\" value=\"2000\"></td></tr>\n\
  <tr><td>Debug level:</td><td><input type=\"text\" name=\"debug\" value=\"0\"></td></tr>\n\
  <tr><td>Client IP address:</td><td><input type=\"text\" name=\"clip\"></td></tr>\n\
  <tr><td>Bind to interface:</td><td><input type=\"text\" name=\"interface\"></td></tr>\n\
  <tr><td>Prefered protocol:</td><td> IPv6<input type=\"radio\" value=\"yes\" name=\"tcp6\">\n\
IPv4<input type=\"radio\" value=\"no\" name=\"tcp6\" checked></td></tr>\n\
  <tr><td>HTTP protocol:</td><td> 1.1<input type=\"radio\" value=\"yes\" name=\"http1.1\" checked>\n\
1.0<input type=\"radio\" value=\"no\" name=\"http1.1\"></td></tr>\n\
\n\
  <tr><td>&nbsp;</td><td>&nbsp;</td></tr>\n\
  <tr><td>&nbsp;</td><td><input type=\"submit\" value=\"Fetch\"></td></tr>\n\
</table>\n\
\n\
</form>\n\
\n\
</body>\n\
</html>\n\
"

#define HTTP_FORM_GEN_ERROR "CGI environment error, can't determine server info required to generate input form.\n"

#ifdef USE_CLOCK_GETTIME
#  define TIME_OUTPUT_FORMAT " %d. type: %d time: %ld.%09ld elap: %ld.%09ld data: %d elap: %.4f\n"
#else
#  define TIME_OUTPUT_FORMAT " %d. type: %d time: %ld.%06ld elap: %ld.%06ld data: %d elap: %.4f\n"
#endif

#define PATT_URI "%uri%"
#define PATT_HOST_COMMENT "%hostcomment%"
#define PATT_HOST_NAME "%webhost%"
#define PATT_USER_AGENT "%agent%"
#define PATT_EXTRA_HEADERS "%exheaders%"
#define PATT_HTTP_PROTOCOL "%httpprot%"

#define USE_HTTP11 0
#define USE_HTTP10 1

#define PROT_HTTP11 "HTTP/1.1"
#define PROT_HTTP10 "HTTP/1.0"

#define EMSG_INVALID_PROXY "Invalid URL given for proxy server."
#define EMSG_INVALID_URL "The URL requested is invalid."
#define EMSG_UNRECOV_NULL_CHECKPOINT "An internal program error lost essential 'checkpoint' data."
#define EMSG_UNRECOV_NULL_ANCHOR "An internal program error lost essential 'anchor' data."
#define EMSG_BAD_IPV6_ADDRESS "The IPv6 address specified is invalid."
#define EMSG_BAD_IPV4_ADDRESS "The IPv4 address specified is invalid."
#define EMSG_NO_IPS_IN_HOSTRECS "No usable IPv4 or IPv6 records found in DNS lookup results."
#define EMSG_NOTHING_TO_LOOKUP "No IP address or hostname to lookup, probably a program error."
#define EMSG_LINGER_WONT "Unable to set 'linger' option on socket."
#define EMSG_NONBLOCK_WONT "Can't configure the socket for non-blocking I/O."
#define EMSG_CONNECT_POLL_TIMEOUT "Connection to webserver (or proxy) timed out."
#define EMSG_REPLY_POLL_TIMEOUT "Timed out waiting for the server to response to request for content."
#define EMSG_READ_POLL_TIMEOUT "Timeout waiting to retrieve content from the server."
#define EMSG_HTTP_HEADER_NO_END "Response from the server did not include a complete HTTP header."
#define EMSG_UNRECOG_OPTIONS "One or more unrecognized options specified."

#define EMSG_TEMP_LOOKUP_NO_SUCH_HOST "No such hostname '%s'."
#define EMSG_TEMP_LOOKUP_BAD_RC "Call to getaddrinfo failed with error, '%s'."

/* --- */

#define SHOW_OPT_IF_DEBUG( PREFIX, NAME) \
if( out->debug_level >= DEBUG_HIGH1 && (co->flags & OP_FL_FOUND)) \
  fprintf( out->info_out, "%sOpt #%d, %s '%s'\n", PREFIX, co->opt_num, NAME, co->val);

#define SPSP( ST) ST ? ST : "\0"

/* --- */

struct http_status_response {
  int code;
  char *version, *reason;
};

struct summary_stats {
  int xfer_sum, payload_sum;
  float lookup_time, lookup_sum, connect_time, connect_sum, request_time, request_sum,
    response_time, response_sum, close_sum, complete_time, complete_sum;
  float packsize_mean, readlag_mean, xfrate_mean,
    packsize_norm_stdev, readlag_norm_stdev, xfrate_norm_stdev,
    packsize_norm_skew, readlag_norm_skew, xfrate_norm_skew,
    packsize_norm_kurt, readlag_norm_kurt, xfrate_norm_kurt,
    packsize_stdev_sum, packsize_skew_sum, packsize_kurt_sum,
    readlag_stdev_sum, readlag_skew_sum, readlag_kurt_sum,
    xfrate_stdev_sum, xfrate_skew_sum, xfrate_kurt_sum;
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
  int conn_port, proxy_port, conn_pthru, pref_protocol, http_protocol;
  struct value_chain *extra_headers;
};

struct output_options {
  int out_html, debug_level;
  FILE *info_out, *err_out;
};

struct display_settings {
  int show_head, show_data, show_timers, show_timerheaders,
    show_packetime, show_help, show_complete, show_number;
  char *line_pref;
};

struct exec_controls {
  int loop_count, loop_pause, conn_timeout, client_ip_type;
  char *client_ip, *bind_interface;
  struct interface_info *device_summ;
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
  time_t wall_start;
};

struct payload_breakout {
  int n_headers, header_size;
  char *content_type, *trans_encoding;
  struct data_block *header_line;
  struct chain_position *head_spot;
  struct http_status_response *response_status;
};

struct plan_data {
  struct target_info *target;
  struct display_settings *disp;
  struct exec_controls *run;
  struct output_options *out;
  struct fetch_status *status;
  struct payload_breakout *partlist;
  struct summary_stats *profile;
};

/* --- */

#define MSG_SHOW_SYNTAX "\
Syntax is: %s <options>\n\
Options are:\n\
    <--header> | <--h>\n\
    <--output> | <--pe>\n\
    <--uri uri>\n\
    <--connhost hostname>\n\
    <--webhost hostname>\n\
    <--url url>\n\
    <--port port>\n\
    <--timers> | <--time>\n\
    <--data>\n\
    <--loop number-of-fetches>\n\
    <--html>\n\
    <--num>\n\
    <--wait seconds>\n\
    <--hfield 'Field: value'>\n\
    <--stderr>\n\
    <--auth user:password>\n\
    <--timeheaders> | <--timeh>\n\
    <--timeout seconds>\n\
    <--proxy hostname:port>\n\
    <--packetime>\n\
    <--clip ip-address>\n\
    <--connthru>\n\
    <--debug debug-level>\n\
    <--tcp4> | <--4> | <--tcp6> | <--6>\n\
    <--http1.0> | <--http1.1>\n\
    <--if device>\n\
"

/* --- */

struct plan_data *figure_out_plan( int *returncode, int narg, char **opts);

struct plan_data *allocate_hf_plan_data();

int find_connection( struct plan_data *plan);

int execute_fetch_plan( struct plan_data *plan);

int construct_request( struct plan_data *plan);

void clear_counters( int *rc, struct plan_data *plan);

void lookup_connect_host( int *rc, struct plan_data *plan);

void connect_to_server( int *rc, struct plan_data *plan);

void send_request( int *rc, struct plan_data *plan);

void wait_for_reply( int *rc, struct plan_data *plan);

void pull_response( int *rc, struct plan_data *plan);

void parse_payload( int *rc, struct plan_data *plan);

void display_output( int *rc, struct plan_data *plan, int iter);

void stats_from_packets( int *rc, struct plan_data *plan, int iter);

int capture_checkpoint( struct fetch_status *anchor, int event_type);

void free_data_block( struct data_block *detail);

int add_data_block( struct ckpt_chain *checkpoint, int len, char *buff);

float calc_time_difference( struct ckpt_entry *start, struct ckpt_entry *end, float clock_res);

struct chain_position *find_header_break( struct ckpt_chain *chain);

void debug_timelog( FILE *out, char *prefix, char *tag);

void display_entry_form();

int split_out_header_lines( struct ckpt_chain *chain, struct payload_breakout *breakout, char *prefix);

char *string_from_data_blocks( struct ckpt_chain *st_block, char *st_pos, struct ckpt_chain *en_block,
  char *en_pos);

int parse_http_response( struct payload_breakout *breakout);

struct http_status_response *parse_http_status( char *line);

char *find_http_header( struct payload_breakout *breakout, char *which);

int find_header_size( struct payload_breakout *breakout, struct ckpt_chain *checkpoint);

/* --- */

#endif
