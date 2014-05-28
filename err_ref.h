#ifndef ___CNAMEjj_ERR_REF_H__

#define ___CNAMEjj_ERR_REF_H__

/*
 * Revision history
 * ----------------
 * 12/19/12
 * -- Subset of "gsub/err_ref.h" with minor changes to make it work outside of
 * --   that context.
 *
 * Todo
 * ----
 */
  
/* --- */

char *cli_strerror( int);

/* --- */

#define RC_NORMAL 0

#define ERR_BAD_HREF 101
#define ERR_BAD_COOKIES 102
#define ERR_NO_FORM_DATA 103
#define ERR_OPEN_APP 104
#define ERR_BAD_PARMS 105
#define ERR_SYS_CALL 106
#define ERR_EMPTY_FILE 107
#define ERR_USE_DOC_ROOT 108
#define ERR_SEL_EXCEPTION 109
#define ERR_SEL_TIMEOUT 110
#define ERR_POLL_TIMEOUT ERR_SEL_TIMEOUT
#define ERR_SEL_SEND 111
#define ERR_SEL_RECV 112
#define ERR_BAD_FORMAT 113
#define ERR_NONEX_FILE 114
#define ERR_MALLOC_FAILED 115
#define ERR_OPEN_FAILED 116
#define ERR_READ_FAILED 117
#define ERR_CONN_FAILED 118
#define ERR_CLOSE_FAILED 119
#define ERR_DISALLOW 120
#define ERR_NO_LOGIN 121
#define ERR_ORACLE_PROB 122
#define ERR_WRITE_FAILED 123
#define ERR_FWRITE_FAILED ERR_WRITE_FAILED
#define ERR_NO_REFER_INFO 124
#define ERR_SEARCH_MAX 125
#define ERR_USER_MISMATCH 126
#define ERR_MISC_ERR 127
#define ERR_RELEASE_FAILED 128
#define ERR_LIMIT_EXCEEDED 129
#define ERR_BAD_DIR 130
#define ERR_MKDIR_FAILED 131
#define ERR_FILE_EXISTS 132
#define ERR_POPEN_FAILED 133
#define ERR_SEEK_FAILED 134
#define ERR_GETHOST_FAILED 135
#define ERR_SOCKET_FAILED 136
#define ERR_FCNTL_FAILED 137
#define ERR_READ_AT_EOF 138
#define ERR_PCLOSE_FAILED 139
#define ERR_BAD_HTTP_RC 140
#define ERR_INVALID_DATA 141
#define ERR_NO_MATCH 142
#define ERR_STAT_FAILED 143
#define ERR_NETWRITE_FAILED 144
#define ERR_USE_DEFAULT 145
#define ERR_SLA_FAILED 146
#define ERR_SIGACTION_FAILED 147
#define ERR_CANT_LOCK 148
#define ERR_CANT_UNLOCK 149
#define ERR_CHMOD_FAILED 150
#define ERR_SYNTAX 151
#define ERR_ALARMCLOCK 152
#define ERR_SHMGET_FAILED 153
#define ERR_SHMAT_FAILED 154
#define ERR_SHMCTL_FAILED 155
#define ERR_IOCTL_FAILED 156
#define ERR_POLL_FAILED 157
#define ERR_UNSUPPORTED 158
#define ERR_OPT_CONFIG 159
#define ERR_NOTHING_LEFT 160
#define ERR_SSL_INIT_FAILED 161
#define ERR_SSL_EOS 162
#define ERR_SSL_ERROR 163
#define ERR_SSLWRITE_FAILED 164
#define ERR_SSLREAD_FAILED 165

#define ERRMSG_BAD_DEST "Destination '%s' invalid, expected 'host:port'."
#define ERRMSG_GETHOST_FAILED "Hostname '%s' could not be resolved to an acceptable IP address."
#define ERRMSG_INET_NTOP "Call to inet_ntop() failed with rc=%d"
#define ERRMSG_SOCKET_CALL "A socket() system call failed with rc=%d"
#define ERRMSG_SETSOCKOPT_CALL "Call to setsockopt() failed with rc=%d"
#define ERRMSG_SENDTO_FAIL "Call to sendto() failed with rc=%d"
#define ERRMSG_SENDTO_PARTIAL "Sent only %d of %d bytes requested."
#define ERRMSG_INET_PTON "Interal error, invalid address family passed to inet_pton() routing, host '%s'."
#define ERRMSG_BIND_FAILED "Call to bind() failed with rc=%d"
#define ERRMSG_SYSCALL2_FAILED "Call to %s() failed with rc=%d"
#define ERRMSG_NO_SUCH_USER "No such user %s."
#define ERRMSG_NO_SUCH_GROUP "No such group %s."
#define ERRMSG_CURR_PWLOOKUP_FAILED "Can't lookup current uid %d, rc=%d."
#define ERRMSG_CURR_GRLOOKUP_FAILED "Can't lookup current group %d, rc=%d."
#define ERRMSG_OPEN_FAILED "Can't open log file '%s', rc=%d."
#define ERRMSG_WRITE_PARTIAL "Partial write to log file, %d of %d bytes written."
#define ERRMSG_UNRECOG_OPTIONS "Unrecognized options '%s' specified."

/* --- */

#ifdef ALLOC_ERR_TEXT

struct err_info {
   int err;
   char *emsg;
};

static struct err_info err_msg_list[] = {
   { ERR_BAD_HREF, "Invalid HREF tag"},
   { ERR_BAD_COOKIES, "Invalid cookie data"},
   { ERR_NO_FORM_DATA, "No form data presented to CGI"},
   { ERR_OPEN_APP, "<<obsolete error code#1>>"},
   { ERR_BAD_PARMS, "Incorrect parameters passed to routine"},
   { ERR_SYS_CALL, "System call returned an error"},
   { ERR_EMPTY_FILE, "File is empty"},
   { ERR_USE_DOC_ROOT,
       "Can't find/change-to current directory, using web document root"},
   { ERR_SEL_EXCEPTION, "Select call 'except' condition occurred"},
   { ERR_SEL_TIMEOUT, "Call to select() or poll() timed out"},
   { ERR_SEL_SEND, "Non-blocking select()->send() call failed"},
   { ERR_SEL_RECV, "Non-blocking select()->recv() call failed"},
   { ERR_BAD_FORMAT, "Bad format"},
   { ERR_NONEX_FILE, "No such file"},
   { ERR_MALLOC_FAILED, "Can't allocate memory (malloc() failed)"},
   { ERR_OPEN_FAILED, "Can't open file/device"},
   { ERR_READ_FAILED, "Call to read() failed"},
   { ERR_CONN_FAILED, "Can't connect to host"},
   { ERR_CLOSE_FAILED, "Can't close file descriptor"},
   { ERR_DISALLOW, "Not allowed"},
   { ERR_NO_LOGIN, "No login info presented by server"},
   { ERR_ORACLE_PROB, "Oracle DB transaction failed, or DB contents in error"},
   { ERR_WRITE_FAILED, "Call to write() failed"},
   { ERR_NO_REFER_INFO, "Can't determine referring page"},
   { ERR_SEARCH_MAX, "Maximum number of search settings reached/exceeded"},
   { ERR_USER_MISMATCH, "Login user doesn't match owner of record in DB"},
   { ERR_MISC_ERR, "Generic minor error handler"},
   { ERR_RELEASE_FAILED, "Can't free Verity search key"},
   { ERR_LIMIT_EXCEEDED, "<<obsolete error code#2>>"},
   { ERR_BAD_DIR, "Invalid/Unusable directory name"},
   { ERR_MKDIR_FAILED, "Can't create directory"},
   { ERR_FILE_EXISTS, "File already exists"},
   { ERR_POPEN_FAILED, "Call to popen() failed"},
   { ERR_SEEK_FAILED, "Call to seek() failed"},
   { ERR_GETHOST_FAILED, "DNS lookup failed"},
   { ERR_SOCKET_FAILED, "Can't create new socket"},
   { ERR_FCNTL_FAILED, "Call to fcntl() failed"},
   { ERR_READ_AT_EOF, "EOF found before end of line character"},
   { ERR_PCLOSE_FAILED, "Can't close pipe"},
   { ERR_BAD_HTTP_RC, "Error code found in HTTP results"},
   { ERR_INVALID_DATA, "Invalid data"},
   { ERR_NO_MATCH, "No match found"},
   { ERR_STAT_FAILED, "Call to stat() failed"},
   { ERR_NETWRITE_FAILED, "NSAPI write failed"},
   { ERR_USE_DEFAULT, "No specific action appropriate, default choice used"},
   { ERR_SLA_FAILED, "Semaphore/Lock arena setup or configuration failed"},
   { ERR_SIGACTION_FAILED, "Call to install/query a signal handler failed"},
   { ERR_CANT_LOCK, "Unable to establish a lock"},
   { ERR_CANT_UNLOCK, "Unable to remove a lock"},
   { ERR_CHMOD_FAILED, "Can't change permissions on file"},
   { ERR_SYNTAX, "Syntax error"},
   { ERR_ALARMCLOCK, "An alarm call timed out"},
   { ERR_SHMGET_FAILED, "Shared memory get/create failed"},
   { ERR_SHMAT_FAILED, "Can't attach shared memory segment"},
   { ERR_SHMCTL_FAILED, "Can't configure shared memory segment"},
   { ERR_IOCTL_FAILED, "Call to ioctl() failed"},
   { ERR_POLL_FAILED, "Call to poll() failed"},
   { ERR_UNSUPPORTED, "Unsupported condition detected"},
   { ERR_OPT_CONFIG, "Configuration error"},
   { ERR_NOTHING_LEFT, "Nothing left to do"},
   { ERR_SSL_INIT_FAILED, "Unable to initialize SSL environment"},
   { ERR_SSL_EOS, "SSL protocol session closed cleanly"},
   { ERR_SSL_ERROR, "SSL protocol error"},
   { ERR_SSLWRITE_FAILED, "SSL write failed"},
   { ERR_SSLREAD_FAILED, "SSL read failed"},
};

/* --- */

#endif

#endif
