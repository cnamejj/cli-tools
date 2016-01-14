#ifndef ___CNAMEjj_QUIPI_H__

#define ___CNAMEjj_QUIPI_H__

/* --- */

#define OK 0

#define NO_HOSTNAME "n/a"
#define NO_IP4_ADDR "0.0.0.0"
#define NO_IP6_ADDR "::0"
#define LOOKUP_FAILED "lookup-failed"
#define LOOKUP_NONAME "no-hostname"

#define ADDR_UNRECOG 0
#define ADDR_IPV4 1
#define ADDR_IPV6 2

/* --- */

#define OP_DEBUG 1
#define OP_HELP 2
#define OP_LOOKUP 3

#define FL_DEBUG "debug"
#define FL_HELP "help"
#define FL_LOOKUP "lookup"

#define DEF_DEBUG "0"
#define DEF_HELP "0"
#define DEF_LOOKUP ""

/* --- */

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

/* --- */

#define HTML_RESP_HEADER "\
Content-Type: text/html\n\
\n\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n\
\"http://www.w3.org/TR/html4/loose.dtd\">\n\
<html><head><title>QUick IP Info</title>\n\
</head>\n\
<body bgcolor=\"#ffffff\">\n\
"

#define HTML_RESP_START "\
<pre>\n\
"

#define HTML_RESP_CLOSE "\
</pre>\n\
</body>\n\
</html>\n\
"

#define HTTP_FORM_TEMPLATE "\
</pre>\n\
<br>Source for QUick-IP-Info (quipi) is available in the <a href=\"https://github.com/cnamejj/cli-tools\">cli-tools</a>\
 repository on GitHub\n\
<br>&nbsp;<br>\n\
<form action=\"http://%s%s%s%s\" method=\"GET\">\n\
<br>List of hostnames and/or IP's\n\
<br><textarea name=\"lookup\" rows=\"10\" cols=\"80\">\n\
</textarea>\n\
<br>Show debug info: <input name=\"debug\" type=\"checkbox\" value=\"15\">\n\
<br><input type=\"submit\" value=\"Lookup\">\n\
</form>\n\
<pre>\n\
"

#define HTTP_FORM_GEN_ERROR "CGI environment error, can't determine server info required to generate input form.\n"

/* --- */

#define MSG_SHOW_SYNTAX "\n\
Syntax is: %s <options> host-or-ip1 host-or-ip2 ...\n\
Options are:\n\
  <--help>\n\
  <--debug level>\n\
"

/* --- */

#endif

