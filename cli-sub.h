#ifndef ___CNAMEjj_CLI_SUB_H__

#define ___CNAMEjj_CLI_SUB_H__

/*
 * Revision history
 * ----------------
 * 8/16/13
 * -- Parts added for CGI support routines
 * 6/10/13 -jj
 * -- Include bits needed for "xml2-util" routines.
 * 1/21/13 -jj
 * -- Combine with other header files for "libCLI" routines, and rename...
 * 12/11/12
 * -- Copied from "junction-ac" code and tweaked to add in error value constants
 *
 * Todo
 * ----
 */
  
#include <sys/stat.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <poll.h>

/* --- */

#define SUBSET_READ  0x4
#define SUBSET_WRITE 0x2
#define SUBSET_EXEC  0x1

#define OP_TYPE_CHAR  1
#define OP_TYPE_FLAG  2
#define OP_TYPE_INT   3
#define OP_TYPE_VOID  4
#define OP_TYPE_FLOAT 5
#define OP_TYPE_LAST  6

#define OP_FL_BLANK   0x00
#define OP_FL_FOUND   0x01
#define OP_FL_SET     0x02
#define OP_FL_INVALID 0x04
#define OP_FL_REPEATS 0x08

#define EXT_FLAG_PREFIX "--"
#define EXT_FLAG_NEGATE "--no-"

#define INT_DISPLAY_LEN 11

#define HEX_BASE 16

#define GNM_FIRST 0
#define GNM_NEXT 1
#define GNM_LAST 2

#define ENV_QUERY_STRING "QUERY_STRING"
#define ENV_SCRIPT_NAME "SCRIPT_NAME"
#define ENV_SERVER_NAME "SERVER_NAME"
#define ENV_SERVER_PORT "SERVER_PORT"

#define CHUNK_MAX 4096
#define BYTE_SIZE 1

#define EOS_CH '\0'
#define PERCENT_CH '%'
#define BLANK_CH ' '
#define PLUS_CH '+'
#define TAB_CH '\t'
#define LF_CH '\n'
#define CR_CH '\r'
#define COLON_CH ':'
#define GT_CH '>'
#define LT_CH '<'
#define DQUOTE_CH '"'
#define AMPER_CH '&'

#define CGI_OPT_DELIM '&'
#define CGI_OPT_EQUAL '='

#define IS_SET_TRUE 1
#define IS_SET_FALSE 0
#define IS_SET_INVALID -1

#define OP_IS_SET_TRUE1 "1"
#define OP_IS_SET_TRUE2 "ON"
#define OP_IS_SET_TRUE3 "YES"
#define OP_IS_SET_FALSE1 "0"
#define OP_IS_SET_FALSE2 "OFF"
#define OP_IS_SET_FALSE3 "NO"

#define COLON_ST ":"
#define EMPTY_ST ""

#define DEFAULT_HTTP_PORT "80"

#define NO_PORT -1
#define NO_SOCK -1

#define URL_VALID 1
#define URL_ERROR 0

#define PROTOCOL_DELIM "://"
#define DOMAIN_DELIM "/"
#define PORT_DELIM ":"
#define QUERY_DELIM "?"
#define USER_DELIM "@"
#define IPV6_PORT_DELIM "#"

#define BASE10 10

#define SC_FORM_PATT_SERVER "<:SERVER:>"
#define SC_FORM_PATT_SCRIPT "<:SCRIPT:>"
#define SC_FORM_PATT_PORT "<:PORT:>"

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

#define POLL_EVENTS_READ POLLIN | POLLRDNORM | POLLPRI
#define POLL_EVENTS_WRITE POLLOUT | POLLWRNORM
#define POLL_EVENTS_ANY POLL_EVENTS_READ | POLL_EVENTS_WRITE | POLLERR | POLLHUP | POLLNVAL | POLLRDBAND | POLLWRBAND
#define POLL_EVENTS_ERROR POLLERR | POLLHUP | POLLNVAL

#ifdef linux
#define SOCKET_OPTS SOCK_STREAM | SOCK_NONBLOCK
#else
#define SOCKET_OPTS SOCK_STREAM
#endif

#define DEF_CLIENT_PORT 80

#define SSL_PROT_PREFIX "https"
#define CTX_MODES SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER
#define SSL_MAX_READ_AHEAD 64 * 1024
#define SSL_TRUSTED_CERT_PATH "/etc/ssl/certs"


/* --- */

struct value_chain {
    unsigned int opt_num, flags;
    char *val;
    void *parsed;
    struct value_chain *next;
};

struct option_set {
   unsigned int num, type, flags;
   char *name, *val, *def;
   void *parsed;
   int opt_num;
};

struct word_chain {
   char *opt;
   struct word_chain *next;
};

struct word_list {
   int count;
   char **words;
};

struct sub_list {
   char *from, *to;
   struct sub_list *next;
};

struct data_chunk {
   char *data;
   int size;
   struct data_chunk *next;
};

struct url_breakout {
    int status, port, use_ssl;
    char *protocol, *target, *user, *host, *ip4, *ip6, *uri, *query;
};

struct interface_info {
    char *name;
    unsigned int flags;
    struct sockaddr *addr;
};

struct http_status_response {
  int code;
  char *version, *reason;
};

/* --- */

struct word_chain *parse_command_options( int *rc, struct option_set *plist, int nopt, int narg, char **opts);

struct option_set *get_matching_option( int flag_num, struct option_set *opset, int nflags);

void print_parse_summary( struct word_chain *, struct option_set *, int);

mode_t convert_to_mode( int);

char *gsub_string( int *rc, char *template, struct sub_list *patts);

char *get_groupname( int *rc, uid_t gid);

char *get_username( int *rc, uid_t uid);

int switch_run_user( char *user);

int switch_run_group( char *group);

char *int_to_str( char *buff, int buff_len, int source, char *format);

char *build_syscall_errmsg( char *syscall, int sysrc);

char *hexdigits_to_string( int *rc, int *msglen, char *hex);

xmlNodePtr search_node_list( xmlNodePtr head, char *node_name, char *attrib_name, char *attrib_val);

char *dup_attrib_value( int *rc, xmlNodePtr curr, char *attrib_name);

char *get_attrib_value( int *rc, xmlNodePtr curr, char *attrib_name);

xmlAttr *search_attrib_list( xmlAttr *head, char *attrib_name, char *attrib_val);

xmlNodePtr get_node_match( xmlNodePtr head, char *node_name, int match_type);

xmlNodePtr source_config_file( int *rc, char *config);

int called_as_cgi();

struct data_chunk *allocate_one_chunk( struct data_chunk *current, int max_chunk_length);

void free_chunk_chain( struct data_chunk *current);

char *get_cgi_data( int *rc);

char *decode_url_str( char *encoded);

int process_parsed_command_options( struct option_set *plist, int nopt, int narg, char **opts);

struct word_chain *add_to_word_chain( struct word_chain *anchor, char *data);

int is_option_set( char *setting);

struct word_chain *parse_cgi_options( int *rc, struct option_set *plist,
  int nopt, char *cgi_data);

struct value_chain *add_option_to_chain( struct value_chain *spot, int opt_num, unsigned int flag,
  char *val, void *parsed);

struct option_set *cond_get_matching_option( int *rc, int which_flag, struct option_set *opset, int nflags);

struct url_breakout *alloc_url_breakout();

void free_url_breakout( struct url_breakout *plan);

struct url_breakout *parse_url_string( char *url);

int is_ipv4_address( char *ip);

int is_ipv6_address( char *ip);

void print_option_settings( FILE *out, int nflags, struct option_set *opset);

char *dup_memory( char *start, char *end);

float get_scaled_number( char *mark, float figure);

char *sys_call_fail_msg( const char *routine);

char *errmsg_with_string( const char *emsg_template, const char *detail);

struct interface_info *get_matching_interface( const char *if_name, int protocol, unsigned int flags);

int is_reserved_uri_char( int spot);

struct http_status_response *parse_http_status( char *line);

char *construct_entry_form( char *template);

int connect_host( int *rc, char *host, int port, int timeout, int protocol);

int wait_until_sock_ready( int sock, int event, int max_wait);

/* --- */

#endif
