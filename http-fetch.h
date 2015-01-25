#ifndef ___CNAMEjj_HTTP_FETCH_H__

#define ___CNAMEjj_HTTP_FETCH_H__

/* --- */

#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <openssl/ssl.h>
#include <openssl/engine.h>

#ifdef DEBUG_MALLOC
#include "bug_malloc.h"
#endif

/* --- */

#ifdef USE_CLOCK_GETTIME
#  include <time.h>
#  define FRAC_RESOLUTION 0.000000001
#else
#  define FRAC_RESOLUTION 0.000001
#endif

#pragma GCC diagnostic ignored "-Woverlength-strings"

/* --- */

#define CH_OPT_NO_VALUE -999999

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
#define FL_MAX_REDIRECT   "redirect"
#define FL_SSL_INSECURE   "insecure"
#define FL_SHOW_SVG       "graph"
#define FL_SVG_FILE       "grout"
#define FL_SVG_STYLE      "grstyle"

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
#define OP_MAX_REDIRECT   29
#define OP_SSL_INSECURE   30
#define OP_SHOW_SVG       31
#define OP_SVG_FILE       32
#define OP_SVG_STYLE      33

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
#define DEF_MAX_REDIRECT   "0"
#define DEF_SSL_INSECURE   "0"
#define DEF_SHOW_SVG       "0"
#define DEF_SVG_FILE       "-"
#define DEF_SVG_STYLE      "light"

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

#define DEF_CONTENT_TYPE "text/html"

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

#define HTML_RESP_INLINE_IMAGE_START "\n<br><img src=\"data:%s,"
#define HTML_RESP_INLINE_IMAGE_END "\">\n<br>\n"

#define HTML_PREFORMAT_START "<pre>"
#define HTML_PREFORMAT_END "</pre>"
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
#define DEFAULT_SSL_FETCH_PORT 443

#define ROOT_URI "/"
#define INVALID_IP "Invalid-IP"
#define UNKNOWN_URL "--unknown-url--"

#define SVG_STYLE_LIGHT "light"
#define SVG_STYLE_DARK "dark"

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

#define LS_START           0x00000
#define LS_PARSED_OPTIONS  0x00001
#define LS_FIND_CONNECTION 0x00002
#define LS_GEN_REQUEST     0x00004
#define LS_CONNECT_LOOKUP  0x00008
#define LS_ESTAB_CONNECT   0x00010
#define LS_SENT_REQUEST    0x00020
#define LS_READ_READY      0x00040
#define LS_GOT_RESPONSE    0x00080
#define LS_USE_GAI_ERRNO   0x00100
#define LS_CKPT_SETUP      0x00200
#define LS_NO_CONNECTION   0x00400
#define LS_NO_REQUEST      0x00800
#define LS_NO_RESPONSE     0x01000
#define LS_NO_PAYLOAD      0x02000
#define LS_NO_DNS_CONNECT  0x04000
#define LS_HTML_FORM_SENT  0x08000
#define LS_SSL_SHAKE_DONE  0x10000

#define EVENT_BLANK           0
#define EVENT_START_FETCH     1
#define EVENT_DNS_LOOKUP      2
#define EVENT_CONNECT_SERVER  3
#define EVENT_REQUEST_SENT    4
#define EVENT_FIRST_RESPONSE  5
#define EVENT_READ_PACKET     6
#define EVENT_READ_ALL_DATA   7
#define EVENT_SSL_NET_READ    8
#define EVENT_SSL_NET_WRITE   9
#define EVENT_SSL_HANDSHAKE  10
#define EVENT_SSL_NEG_READ   11

#define EVNAME_BLANK          "unknown event"
#define EVNAME_START_FETCH    "start fetch"
#define EVNAME_DNS_LOOKUP     "DNS lookup"
#define EVNAME_CONNECT_SERVER "connect server"
#define EVNAME_REQUEST_SENT   "request sent"
#define EVNAME_FIRST_RESPONSE "first response"
#define EVNAME_READ_PACKET    "application read"
#define EVNAME_READ_ALL_DATA  "all data received"
#define EVNAME_SSL_NET_READ   "network read"
#define EVNAME_SSL_NET_WRITE  "network write"
#define EVNAME_SSL_HANDSHAKE  "ssl handshake"
#define EVNAME_SSL_NEG_READ   "pre-SSL neg read"

#define SSLACT_NORMAL        0
#define SSLACT_ERR_FATAL     1
#define SSLACT_RETRY         2
#define SSLACT_ERR_HANDSHAKE 3
#define SSLACT_ERR_CLOSED    4
#define SSLACT_WRITE         5
#define SSLACT_READ          6

#define MSTONE_DNS "DNS"
#define MSTONE_CONN "Connect"
#define MSTONE_SSL "SSL"
#define MSTONE_SEND "Send"
#define MSTONE_1STREAD "1stRead"
#define MSTONE_ALLDATA "Complete"

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
-Date--Time-     ------------- Elapsed Time -------------- -------- Transfer --------- -- Received Packet Size --- ---- Inter-Packet Lag ----- -- Per-Packet Xfer Rate --- ---\
"
#define TIME_SUMMARY_HEADER_2 "\
YrMnDyHrMnSe HRC   DNS  Conn   SSL  Send 1stRD Close Total AllByt PayByt Tot#/S Dat#/S   StDev  Skewness  Kurtosis   StDev  Skewness  Kurtosis   StDev  Skewness  Kurtosis URL\
"
#define TIME_SUMMARY_HEADER_3 "\
------------ --- ----- ----- ----- ----- ----- ----- ----- ------ ------ ------ ------ ------- --------- --------- ------- --------- --------- ------- --------- --------- ---\
"

#define TIME_SUMMARY_HEADER_SEQ_1 "     "
#define TIME_SUMMARY_HEADER_SEQ_2 " Seq "
#define TIME_SUMMARY_HEADER_SEQ_3 "---- "

#define TIME_DISPLAY_FORMAT "%y%m%d%H%M%S"
#define TIME_DISPLAY_SIZE 15


#define HTTP_HEAD_TRANSFER_ENCODING "Transfer-Encoding"
#define HTTP_HEAD_CONTENT_TYPE "Content-Type"
#define HTTP_HEAD_LOCATION "Location"

#define HTTP_RC_MULTI_CH  300
#define HTTP_RC_MOVE_PERM 301
#define HTTP_RC_FOUND     302
#define HTTP_RC_SEE_OTHER 303
#define HTTP_RC_NO_CHANGE 304
#define HTTP_RC_USE_PROXY 305
#define HTTP_RC_SWI_PROXY 306
#define HTTP_RC_TEMP_RED  307
#define HTTP_RC_PERM_RED  308

#define ENC_TYPE_CHUNKED "chunked"
#define CONTENT_IMAGE_PREF "image/"

#define HTML_SCRIPT_FORM_TEMPLATE "\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\
<html><head><title>Http-Fetch tester form</title>\n\
</head>\n\
<body bgcolor=\"#ffffff\">\n\
\n\
<br>Source for HTTP-FETCH is available in the <a href=\"https://github.com/cnamejj/cli-tools\">cli-tools</a>\
 repository on GitHub\n\
<br>&nbsp;<br>\n\
<form action=\"http://<:SERVER:><:PORT:><:SCRIPT:>\" method=\"GET\">\n\
\n\
<!-- These options aren't useful in a CGI: stderr -->\n\
<!-- Options that are't implemented yet: html, auth, proxy, connthru -->\n\
\n\
<table>\n\
  <tr><td colspan=2>What to fetch:</td></tr>\n\
  <tr><td>Full URL:</td><td><input type=\"text\" name=\"url\"></td></tr>\n\
  <tr><td>Connect to host/ip:</td><td><input type=\"text\" name=\"connhost\"></td></tr>\n\
  <tr><td>Name of webserver:</td><td><input type=\"text\" name=\"webhost\"></td></tr>\n\
  <tr><td>Connect to this port:</td><td><input type=\"text\" name=\"port\"></td></tr>\n\
  <tr><td>URI to request:</td><td><input type=\"text\" name=\"uri\"></td></tr>\n\
  <tr><td>Accept insecure SSL certs:</td><td> Yes<input type=\"radio\" value=\"yes\" name=\"insecure\">\n\
No<input type=\"radio\" value=\"no\" name=\"insecure\" checked></td></tr>\n\
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
  <tr><td>Show packet level graphs:</td><td> Yes<input type=\"radio\" value=\"yes\" name=\"graph\" checked>\n\
No<input type=\"radio\" value=\"no\" name=\"graph\"></td></tr>\n\
  <tr><td>Graph style:</td><td> Light<input type=\"radio\" value=\"light\" name=\"grstyle\" checked>\n\
Dark<input type=\"radio\" value=\"dark\" name=\"grstyle\"></td></tr>\n\
\n\
  <tr><td>&nbsp;</td><td>&nbsp;</td></tr>\n\
  <tr><td colspan=2>Recipe:</td></tr>\n\
  <tr><td>Network timeout:</td><td><input type=\"text\" name=\"timeout\" value=\"30000\"></td></tr>\n\
  <tr><td>How many times to fetch:</td><td><input type=\"text\" name=\"loop\" value=\"1\"></td></tr>\n\
  <tr><td>Pause time between fetches:</td><td><input type=\"text\" name=\"wait\" value=\"2000\"></td></tr>\n\
  <tr><td>Debug level:</td><td><input type=\"text\" name=\"debug\" value=\"0\"></td></tr>\n\
  <tr><td>Client IP address:</td><td><input type=\"text\" name=\"clip\"></td></tr>\n\
  <tr><td>Bind to interface:</td><td><input type=\"text\" name=\"interface\"></td></tr>\n\
  <tr><td>Prefered protocol(*):</td><td> IPv6<input type=\"radio\" value=\"yes\" name=\"tcp6\">\n\
IPv4<input type=\"radio\" value=\"no\" name=\"tcp6\" checked></td></tr>\n\
  <tr><td><font size=\"-1\">* EC2 VM's do not support IPV6&nbsp;&nbsp;</font></td></tr>\n\
  <tr><td>HTTP protocol:</td><td> 1.1<input type=\"radio\" value=\"yes\" name=\"http1.1\" checked>\n\
1.0<input type=\"radio\" value=\"no\" name=\"http1.1\"></td></tr>\n\
  <tr><td>Maximum redirects:</td><td><input type=\"text\" name=\"redirect\" value=\"0\"></td></tr>\n\
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

#define HTML_FORM_GEN_ERROR "CGI environment error, can't determine server info required to generate input form.\n"

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

/* --- */

#define GR_PACK_TITLE_LEAD "Packet Receipt - URL: "
#define GR_PACK_XAX_TITLE "Elapsed Time (secs between reads)"
#define GR_PACK_YAX_TITLE "Packetsize"
#define GR_PACK_DATA_LINE_COLOR "#a47e28"
#define GR_PACK_DATA_LINE_ALPHA 0.35

#define GR_ACCDAT_TITLE_LEAD "Data Rec'd - URL: "
#define GR_ACCDAT_XAX_TITLE "Elapsed Time (secs)"
#define GR_ACCDAT_YAX_TITLE "Data Received"

#define GR_FR_PSIZE_TITLE_LEAD "Freq of Packet Sizes - URL: "
#define GR_FR_PSIZE_XAX_TITLE "Packet Size"
#define GR_FR_PSIZE_YAX_TITLE "Frequency"

#define GR_FR_RWAIT_TITLE_LEAD "Freq of Packet Wait - URL: "
#define GR_FR_RWAIT_XAX_TITLE "Read Wait (secs between packets)"
#define GR_FR_RWAIT_YAX_TITLE "Frequency"

#define GR_ALL_XAX_GRIDS 7
#define GR_ALL_YAX_GRIDS 5
#define GR_DATA_META_FORMAT "%%.%df"
#define GR_ALL_CIRC_LINE_SIZE 10
#define GR_ALL_CIRC_RADIUS 22
#define GR_ALL_TEXT_COLOR "#124488"
#define GR_ALL_AXIS_COLOR "#444444"
#define GR_ALL_CHART_COLOR "#FFFFFF"
#define GR_ALL_GRAPH_COLOR "#000000"
#define GR_ALL_CIRC_FILL_COLOR "#088ea0"
#define GR_ALL_CIRC_LINE_COLOR "#e0da24"
#define GR_ALL_DATA_FILL_COLOR "#FFFFFF"
#define GR_ALL_DATA_LINE_COLOR "#744e18"
#define GR_ALL_XGRID_COLOR "#2e2e2e"
#define GR_ALL_YGRID_COLOR "#2e2e2e"
#define GR_ALL_GRAPH_ALPHA 0.05
#define GR_ALL_CIRC_FILL_ALPHA 0.3
#define GR_ALL_CIRC_LINE_ALPHA 0.4
#define GR_ALL_DATA_FILL_ALPHA 0.0
#define GR_ALL_DATA_LINE_ALPHA 0.6
#define GR_MST_LINE_COLOR "#dac032"
#define GR_MST_TEXT_COLOR "#aa6032"

#define GR_MIN_CASES 3
#define GR_FREQ_BRACKETS 96

#define GR_PACK_NOGRAPH "<!-- Unsufficient number of datapoints for packet graph -->\n"
#define GR_ACCDATA_NOGRAPH "<!-- Unsufficient number of datapoints for received bytes graph -->\n"

/* --- */

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
#define EMSG_SSL_SETUP_FAIL "Can't setup SSL session."
#define EMSG_SSL_CONN_FAIL "Failed to complete SSL connection/handshake."
#define EMSG_SSL_HANDSHAKE_FAIL "SSL handshake failed without setting an error."

#define EMSG_TEMP_LOOKUP_NO_SUCH_HOST "No such hostname '%s'."
#define EMSG_TEMP_LOOKUP_BAD_RC "Call to getaddrinfo failed with error, '%s'."

/* --- */

#define SHOW_OPT_IF_DEBUG( PREFIX, NAME) \
if( out->debug_level >= DEBUG_HIGH1 && (co->flags & OP_FL_FOUND)) \
  fprintf( out->info_out, "%sOpt #%d, %s '%s'\n", PREFIX, co->opt_num, NAME, co->val);

#define SPSP( ST) ST ? ST : "\0"

/* --- */

struct summary_stats {
  int xfer_sum, payload_sum, fetch_count;
  float lookup_time, lookup_sum, connect_time, connect_sum, request_time, request_sum,
    response_time, response_sum, close_sum, complete_time, complete_sum, handshake_time,
    handshake_sum;
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
  int conn_port, proxy_port, conn_pthru, pref_protocol,
    http_protocol, use_ssl, insecure_cert;
  struct value_chain *extra_headers;
};

struct output_options {
  int out_html, debug_level;
  char *svg_file, *svg_style;
  FILE *info_out, *err_out, *svg_out;
};

struct display_settings {
  int show_head, show_data, show_timers, show_timerheaders,
    show_packetime, show_help, show_complete, show_number,
    show_svg;
  char *line_pref;
};

struct exec_controls {
  int loop_count, loop_pause, conn_timeout, client_ip_type, redirect_depth;
  char *client_ip, *bind_interface;
  struct interface_info *device_summ;
};

struct fetch_status {
  long response_len;
  int ip_type, conn_sock, request_len, wait_timeout, last_state,
    end_errno;
  char *err_msg, *request, *primary_request, *redirect_request;
  float clock_res;
  struct sockaddr_in sock4;
  struct sockaddr_in6 sock6;
  struct ckpt_chain *checkpoint, *lastcheck;
  time_t wall_start;
  SSL_CTX *ssl_context;
  SSL *ssl_box;
};

struct payload_breakout {
  int n_headers, header_size;
  char *content_type, *trans_encoding;
  struct data_block *header_line;
  struct chain_position *head_spot;
  struct http_status_response *response_status;
};

struct plan_data {
  struct target_info *target, *redirect;
  struct display_settings *disp;
  struct exec_controls *run;
  struct output_options *out;
  struct fetch_status *status;
  struct payload_breakout *partlist;
  struct summary_stats *profile;
};

struct stat_work {
    int samples, *packsize;
    long packsize_sum;
    float *xfrate, *readlag,
      packsize_max, readlag_max, xfrate_max,
      packsize_mean, readlag_mean, xfrate_mean,
      packsize_norm_stdev, readlag_norm_stdev, xfrate_norm_stdev,
      packsize_norm_skew, readlag_norm_skew, xfrate_norm_skew,
      packsize_norm_kurt, readlag_norm_kurt, xfrate_norm_kurt,
      readlag_sum, xfrate_sum;
};

struct chart_options {
    int xmin_hard, xmin_soft, xmax_hard, xmax_soft, 
      ymin_hard, ymin_soft, ymax_hard, ymax_soft;
    float data_line_alpha;
    char *data_line_color;
    struct milestone *mstone;
};

struct milestone {
    float offset;
    char *label;
    struct milestone *next;
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
    <--redirect max-levels>\n\
    <--insecure>\n\
    <--graph>\n\
    <--grout file-name.html>\n\
    <--grstyle dark> | <--grstyle light>>\n\
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

int add_datalen_block( struct ckpt_chain *checkpoint, int len);

float calc_time_difference( struct ckpt_entry *start, struct ckpt_entry *end, float clock_res);

struct chain_position *find_header_break( struct ckpt_chain *chain);

void debug_timelog( FILE *out, char *prefix, char *tag);

void display_entry_form();

int split_out_header_lines( struct ckpt_chain *chain, struct payload_breakout *breakout, char *prefix);

char *string_from_data_blocks( struct ckpt_chain *st_block, char *st_pos, struct ckpt_chain *en_block,
  char *en_pos);

int parse_http_response( struct payload_breakout *breakout);

char *find_http_header( struct payload_breakout *breakout, char *which);

int find_header_size( struct payload_breakout *breakout, struct ckpt_chain *checkpoint);

char *get_redirect_location( int *rc, struct plan_data *plan);

void map_target_to_target( struct plan_data *plan);

void display_average_stats( int *rc, struct plan_data *plan);

void setup_ssl_env( int *rc, struct plan_data *plan);

void ssl_handshake( int *rc, struct plan_data *plan);

SSL_CTX *init_ssl_context(int (*callback)(int, X509_STORE_CTX *));

SSL *map_sock_to_ssl(int sock, SSL_CTX *context, long (*callback)(struct bio_st *, int, const char *, int, long, long));

int verify_ssl_callback(int ok, X509_STORE_CTX *context);

long bio_ssl_callback(BIO *bn, int flags, const char *buff, int blen, long ignore, long ret);

int handle_ssl_error( int *sslerr, SSL *ssl, int io_rc, int sock, int max_wait);

struct plan_data *register_current_plan( struct plan_data *update);

int stash_ssl_err_info( struct fetch_status *fetch, unsigned long sslerr);

void calc_xfrates( int want_event, struct fetch_status *status, struct stat_work *swork);

void calc_standard_moments( struct fetch_status *status, struct stat_work *swork);

struct stat_work *alloc_stat_work();

char *hf_generate_graph( int *rc, int cases, float *xdata, float *ydata, char *style, char *title,
  char *xax_title, char *yax_title, struct chart_options *chopt);

char *make_packet_graph( int *rc, char *url, char *style, int ssl, struct fetch_status *fetch);

char *make_accdata_graph( int *rc, char *url, char *style, int ssl, struct fetch_status *fetch);

char *make_psize_freq_graph( int *rc, char *url, char *style, int ssl, struct fetch_status *fetch);

char *make_rwait_freq_graph( int *rc, char *url, char *style, int ssl, struct fetch_status *fetch);

struct chart_options *alloc_chart_options();

void free_chart_options( struct chart_options *chopt);

void free_target_data( struct target_info *targ);

void free_display_data( struct display_settings *disp);

void free_interface_data( struct interface_info *dinfo);

void free_exec_data( struct exec_controls *run);

void free_output_data( struct output_options *out);

void free_fstat_data( struct fetch_status *fetch);

void free_chain_position( struct chain_position *cpos);

void free_http_status_response( struct http_status_response *resp);

void free_payload_data( struct payload_breakout *bout);

void free_hf_plan_data( struct plan_data *plan);

void free_checkpoint_chain( struct ckpt_chain *chain);

void clear_parsed_headers( int *rc, struct plan_data *plan);

void free_payload_references( struct payload_breakout *bout);

/* --- */

#endif
