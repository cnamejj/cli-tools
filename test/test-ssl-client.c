#include <openssl/ssl.h>
#include <openssl/engine.h>
#include <openssl/x509v3.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include "../cli-sub.h"
#include "../err_ref.h"

/* --- */

/* #define CTX_MODES SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER */
#define PAUSE 50000
#define POLL_TIMEOUT 10000
#define BUFFSIZE 2048

/* #define SSL_TRUSTED_CERT_PATH "/etc/ssl/certs" */

#define ERR_OUT stdout
#define BUFFLEN 255
/* #define MAX_READ_AHEAD 64 * 1024 */

#define NORMAL_REQ_TEMPLATE "\
GET /%s HTTP/1.1\r\n\
Host: %s\r\n\
User-Agent: Mozilla/5.0 Gecko/20100101 Firefox/23.0\r\n\
Accept: */*\r\n\
Connection: close\r\n\
DNT: 1\r\n\
\r\n"

#define CURL_REQ_TEMPLATE "\
GET /%s HTTP/1.1\r\n\
User-Agent: curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3\r\n\
Host: %s\r\n\
Accept: */*\r\n\
\r\n"

#define SEAMONKEY_REQ_TEMPLATE "\
GET /%s HTTP/1.1\r\n\
Host: %s\r\n\
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.7; rv:29.0) Gecko/20100101 Firefox/29.0 SeaMonkey/2.26\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n\
Accept-Language: en-US,en;q=0.5\r\n\
Accept-Encoding: gzip, deflate\r\n\
DNT: 1\r\n\
Connection: keep-alive\r\n\
\r\n"

/* Some sample browser id strings.
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_5) AppleWebKit/537.75.14 (KHTML, like Gecko) Version/6.1.3 Safari/537.75.14\r\n\
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.7; rv:28.0) Gecko/20100101 Firefox/28.0\r\n\
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.7; rv:29.0) Gecko/20100101 Firefox/29.0 SeaMonkey/2.26\r\n\
 */

#define DEBUG_REQ_TEMPLATE "\
GET /%s HTTP/1.1\r\n\
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_5) AppleWebKit/537.75.14 (KHTML, like Gecko) Version/6.1.3 Safari/537.75.14\r\n\
Host: %s\r\n\
Accept: */*\r\n\
Connection: close\r\n\
DNT: 1\r\n\
\r\n"

#define REQ_TEMPLATE DEBUG_REQ_TEMPLATE

/* --- */

void err_exit(char *msg)

{
    fprintf(ERR_OUT, "Err: %s\n", msg);
    exit(1);
}

#ifdef USE_OLD_CODE
/* --- */

int wait_until_sock_ready(sock, event, timeout)

{
    int rc = 0;
    struct pollfd pset;

    pset.fd = sock;
    pset.events = event;
    pset.revents = 0;
    
    rc = poll(&pset, 1, timeout);
    printf("dbg:: WUSR: poll, event=%04x, rc=%d\n", event, rc);

    return(rc);
}
#endif

/* --- */

long handle_bio_callback(BIO *bn, int flags, const char *buff, int blen, long ignore, long ret)

{
/*    int lasterr;
 *    lasterr = errno;
 *
 *    printf("BIO-event, flags=%d %x err=%d\n", flags, flags, lasterr);
 */

    if(flags == (BIO_CB_RETURN | BIO_CB_READ) )
    {
        printf("BIO-read, len=%ld\n", ret);
    }
    else if(flags == (BIO_CB_RETURN | BIO_CB_WRITE) )
    {
        printf("BIO-write, len=%ld\n", ret);
    }

/*
    fflush(stdout);
    (void) BIO_debug_callback(bn, flags, buff, blen, ignore, ret);
    fflush(stderr);
 */

    return(ret);
}

/* --- */

int verify_callback(int ok, X509_STORE_CTX *context)

{
    int rc = ok, namelen, done, pos, x509_err, x509_depth, exn, nid_num, excount, altcount, alt;
    unsigned char *nameval = 0;
    const char *x509_emsg = 0;
    X509 *cert;
    X509_NAME *subject = 0;
    X509_NAME_ENTRY *common = 0;
    X509_EXTENSION *cex = 0;
    ASN1_STRING *cdata = 0;
    ASN1_IA5STRING *altdns = 0;
    GENERAL_NAME *altval = 0;
    STACK_OF(GENERAL_NAME) *altnames = 0;

    x509_err = X509_STORE_CTX_get_error(context);
    x509_depth = X509_STORE_CTX_get_error_depth(context);
    printf("\n\ndbg:: VC: called with ok=%d, xerr=%d, xdep=%d\n", ok, x509_err, x509_depth);
    if(!ok)
    {
        x509_emsg = X509_verify_cert_error_string(x509_err);
        printf("dbg:: Error message '%s'\n", x509_emsg);
    }

    cert = X509_STORE_CTX_get_current_cert(context);
    if(cert)
    {
        printf("dbg:: VC: 1. got a cert\n");
        subject = X509_get_subject_name(cert);
        done = 0;
        pos = -1;
        for( ; !done; )
        {
            printf("dbg:: VC: 2. top loop, pos=%d\n", pos);
            pos = X509_NAME_get_index_by_NID(subject, NID_commonName, pos);
            if(pos == -1) done = 1;
            else
            {
                common = X509_NAME_get_entry(subject, pos);
                if(common)
                {
                    printf("dbg:: VC: 2. got name entry, pos=%d\n", pos);
                    cdata = X509_NAME_ENTRY_get_data(common);
                    if(cdata)
                    {
                        printf("dbg:: VC: 2. decoding name\n");
                        namelen = ASN1_STRING_length(cdata);
                        nameval = ASN1_STRING_data(cdata);
                        printf("Common name: '");
                        fwrite(nameval, 1, namelen, stdout);
                        printf("'\n");
		    }
		}
	    }
	}

        excount = X509_get_ext_count(cert);
        printf("dbg:: VC: ext count=%d\n", excount);
        for(exn = 0; exn < excount; exn++)
        {
            cex = X509_get_ext(cert, exn);
            nid_num = OBJ_obj2nid(cex->object);
            printf("dbg:: VC: ext, nid=%d name'%s'\n", nid_num, OBJ_nid2ln(nid_num));
            printf("dbg:: VC: ext, val'");
            X509V3_EXT_print_fp(stdout, cex, 0, 1);
            printf("'\n");
	}

        pos = -1;
        altnames = X509_get_ext_d2i(cert, NID_subject_alt_name, 0, &pos);

        for( ; altnames; )
        {
            altcount = sk_GENERAL_NAME_num(altnames);
            for( alt = 0; alt < altcount; alt++)
            {
                altval = sk_GENERAL_NAME_value(altnames, alt);
                printf("dbg:: VC: altloop, off=%d, type=%d ? %d\n", alt, altval->type, GEN_DNS);
/*                if(altval->type == GEN_DNS) printf("dbg:: VC: altloop, off=%d, name'%s'\n", alt, altval->d.ia5->data); */
                altdns = (ASN1_IA5STRING *) GENERAL_NAME_get0_value(altval, 0);
                if(altval->type == GEN_DNS) printf("dbg:: VC: altloop, off=%d, name'%s'\n", alt, altdns->data);
	    }
            GENERAL_NAMES_free(altnames);
            altnames = X509_get_ext_d2i(cert, NID_subject_alt_name, 0, &pos);
	}

        if(altnames) GENERAL_NAMES_free(altnames);
    }

    rc = 1;
    return(rc);
}

/* --- */

int handle_ssl_error(SSL *ssl, int io_rc, int sock, int max_wait)

{
    int off = 0, sysrc, sslerr, rc = 0, event;

    sslerr = SSL_get_error(ssl, io_rc);
    if(sslerr == SSL_ERROR_WANT_READ || sslerr == SSL_ERROR_WANT_WRITE)
    {
        if(sslerr == SSL_ERROR_WANT_READ) event = POLL_EVENTS_READ;
        else event = POLL_EVENTS_WRITE;

/* max_wait should be decremented each time through the loop since it's overall not "per poll */
        fprintf(ERR_OUT, "Info: %d. Wait for %d event then re-try SSL I/O.\n", ++off, sslerr);
        sysrc = wait_until_sock_ready(sock, event, max_wait);
        if(!sysrc) err_exit("Call to poll() timed out");
        else if(sysrc == -1) err_exit("Call to poll() failed");
    }
    else if(sslerr == SSL_ERROR_WANT_CONNECT || sslerr == SSL_ERROR_WANT_ACCEPT ||
      sslerr == SSL_ERROR_WANT_X509_LOOKUP)
    {
        fprintf(ERR_OUT, "Info: %d. Need to re-try SSL_connect(), code=%d.\n", ++off, sslerr);
    }
    else if(sslerr == SSL_ERROR_SYSCALL)
    {
        sysrc = errno;

        if(sysrc == EAGAIN || sysrc == EWOULDBLOCK || sysrc == EBUSY ||
          sysrc == EINPROGRESS || sysrc == EINTR || sysrc == ERESTART)
        {
            fprintf(ERR_OUT, "Info: %d. Recoverable system error %d, re-try SSL_connect()\n", ++off, sysrc);
        }
        else
        {
            fprintf(ERR_OUT, "Err: Last error (%d) %s\n", sysrc, strerror(sysrc));
fflush(stdout);
/* sleep(5); */
            err_exit("SSL handshake failed with SSL_ERROR_SYSCALL");
        }
    }
    else
    {
        fprintf(ERR_OUT, "Err: rc=%d/%d\n", io_rc, sslerr);
        err_exit("SSL handshake failed");
    }

    return(rc);
}

/* --- */

int do_ssl_handshake(SSL *ssl, int sock)

{
    int done = 0, rc = 0, sysrc;

    for(; !done; )
    {
        sysrc = SSL_connect(ssl);
        if(sysrc == 1) done = 1;
        else done = handle_ssl_error(ssl, sysrc, sock, POLL_TIMEOUT);
    }

    return(rc);
}

/* --- */

int main(int narg, char **opts)

{
    int rc = 0, sysrc = 0, port = 443, sock = NO_SOCK, timeout = 10000,
      done, sslerr, off, http_comm_size, left, outlen;
/*  int is_shut; */
    char *host = 0, *http_comm, *st, *buff, *uri = 0;
/*    struct pollfd pset; */
    SSL_CTX *context = 0;
    SSL *ssl = 0;
    const char *qerr_file;
    unsigned long qerr;
    int qerr_line;

    qerr_file = (char *) malloc(2048);

    if(narg < 2)
    {
        printf( "Syntax: %s host|ip <port> <uri>\n", opts[0]);
        exit(1);
    }

    host = opts[1];

    if(narg >= 3) port = atoi(opts[2]);
    if(narg >= 4) uri = opts[3];

    if(!uri) uri = "";
    else if(!*uri) uri = "";
    else if(*uri == '/') uri++;

    /* --- */

    http_comm_size = strlen(host) + strlen(uri) + (sizeof REQ_TEMPLATE) + 1;
    http_comm = (char *) malloc(http_comm_size);
    if(!http_comm) err_exit("Call to malloc() failed");

    sysrc = snprintf(http_comm, http_comm_size, REQ_TEMPLATE, uri, host);
    if(sysrc < 0) err_exit("Can't construct HTTP request.");

    http_comm_size = strlen(http_comm);

    printf("dbg:: request:");
    for(st=http_comm; *st; st++)
      if(*st == '\r') printf("\\r");
      else if(*st == '\n') printf("\\n");
      else putc(*st, stdout);
    printf("\n\n");
    fflush(stdout);

    /* --- */

    buff = (char *) malloc(BUFFSIZE);
    if(!buff) err_exit("Call to malloc() failed");

    /* --- */

    sock = connect_host(&rc, host, port, timeout, AF_INET);

    context = init_ssl_context(verify_callback);
    if(!context) err_exit("No ciphers could be loaded");

    fprintf(ERR_OUT, "dbg:: post-conn host'%s' port=%d timeout=%d sock=%d rc=%d\n",
      host, port, timeout, sock, rc);

    ssl = map_sock_to_ssl(sock, context, handle_bio_callback);

    rc = do_ssl_handshake(ssl, sock);
    
    /* --- */

    done = 0;
    for(; !done; )
    {
        sysrc = wait_until_sock_ready(sock, POLL_EVENTS_WRITE, POLL_TIMEOUT);
        if(!sysrc) err_exit("Call to poll() timed out");
        else if(sysrc == -1) err_exit("Call to poll() failed");

        sysrc = SSL_write(ssl, http_comm, http_comm_size);
        if(sysrc > 0)
        {
            left = http_comm_size - sysrc;
            if(left <= 0) done = 1;
            else
            {
                fprintf(ERR_OUT, "Info: Partial write, need to send %d more bytes.\n", left);
                st = http_comm + sysrc;
                for(off = 0; *st; off++)
                {
                    *(http_comm + off) = *st;
                    st++;
		}
                http_comm_size = strlen(http_comm);
	    }
	}
        else
        {
            done = handle_ssl_error(ssl, sysrc, sock, POLL_TIMEOUT);
            if(done) err_exit("SSL error sending HTTP request");
	}
    }

    /* --- */

    done = 0;
    for(; !done; )
    {
        qerr = ERR_get_error_line(&qerr_file, &qerr_line);
        for(; qerr; )
        {
            printf("dbg:: ErrQueue: rc=%ld, line=%d, file'%s'\n", qerr, qerr_line, qerr_file);
            qerr = ERR_get_error_line(&qerr_file, &qerr_line);
	}

        if(SSL_get_shutdown(ssl) & SSL_RECEIVED_SHUTDOWN)
        {
            done = 1;
            printf("dbg:: pre-read, received SSL shutdown\n");
            SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
	}
        else
        {
            sysrc = wait_until_sock_ready(sock, POLL_EVENTS_READ, POLL_TIMEOUT);
            if(!sysrc) err_exit("Call to poll() timed out");
            else if(sysrc == -1) err_exit("Call to poll() failed");

fprintf(ERR_OUT, "dbg:: pre-SSL_read, errno=%d\n", errno);
            sysrc = SSL_read(ssl, buff, BUFFSIZE);
            sslerr = SSL_get_error(ssl, sysrc);
fprintf(ERR_OUT, "dbg:: post-SSL_read, rc=%d, sslerr=%d, errno=%d\n", sysrc, sslerr, errno);
            if(sysrc > 0)
            {
                outlen = sysrc;
                for(; outlen;)
                {
                    sysrc = fwrite(buff, 1, outlen, stdout);
                    if(sysrc > 0)
                    {
                        outlen -= sysrc;
                        if(outlen <= 0) outlen = 0;
                        else
                        {
                            st = buff + sysrc;
                            for(off = 0; off < outlen; off++) *(buff + off) = *(st + off);
                        }
                    }
                    else err_exit("Call to fwrite() failed");
                }
            }
            else
            {
                sslerr = SSL_get_error(ssl, sysrc);
                if(sslerr == SSL_ERROR_ZERO_RETURN)
                {
                    fprintf(ERR_OUT, "1. SSL_ERROR_ZERO_RETURN, clean shutdown\n");
                    SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
                    SSL_free(ssl);
                    done = 1;
                }
                else
                {
                    done = handle_ssl_error(ssl, sysrc, sock, POLL_TIMEOUT);
                    if(done) err_exit("Unrecoverable error in read loop");
		}
	    }
	}
    }

    /* --- */

    printf("dbg:: Close up and go home.\n");
    fflush(stdout);

    if(sock != NO_SOCK) close(sock);

    /* --- */

    exit(rc);
}
