
#define ERR_EXIT(MSG) \
{ \
    fprintf(stderr, "Error: rc=%d %s\n", errno, MSG); \
    exit(1); \
}

/* --- */

#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "http-fetch.h"
#include "err_ref.h"
#ifdef S2N_SUPPORT
    #include "cli-sub.h"
#endif

/* --- */

void ssl_handshake( int *rc, struct plan_data *plan)

{
    int sock, io_rc, err = 0, done = 0, ret, sslerr;
#ifdef S2N_SUPPORT
    int pending, sysrc;
#endif
    unsigned long hold_err;
    struct target_info *targ = 0;
    struct fetch_status *fetch;
    struct exec_controls *runex = 0;
    SSL *ssl;
#ifdef S2N_SUPPORT
    s2n_blocked_status s2n_stat;
    time_t now, deadline;
#endif

    ENTER( "ssl_handshake")

    if( *rc == RC_NORMAL)
    {
        fetch = plan->status;
        runex = plan->run;
        if( plan->redirect) if( plan->redirect->conn_url) if( *plan->redirect->conn_url) targ = plan->redirect;
        if( !targ) targ = plan->target;

        if( targ->use_ssl && !targ->use_s2n)
        {
            sock = fetch->conn_sock;
            ssl = map_sock_to_ssl( sock, fetch->ssl_context, bio_ssl_callback);
            if( ssl) fetch->ssl_box = ssl;
            else
            {
                err = 1;
                *rc = ERR_SSL_ERROR;
                hold_err = ERR_peek_error();
                if( hold_err)
                {
                    fetch->end_errno = hold_err;
                    (void) stash_ssl_err_info( fetch, hold_err);
		}
                else
                {
                    fetch->end_errno = errno;
                    fetch->err_msg = strdup( EMSG_SSL_SETUP_FAIL);
		}
	    }

/* NOTE: timeout remainder should be re-calc'd each time through the loop 
 * but leaving it simple for now, until the basic logic is working.
 */
            for( ; !done && !err; )
            {
                io_rc = SSL_connect( ssl);
                if( io_rc == 1)
                {
                    done = 1;
                    *rc = capture_checkpoint( fetch, EVENT_SSL_HANDSHAKE);
                    if( *rc == RC_NORMAL) fetch->last_state |= LS_SSL_SHAKE_DONE;
		}
                else
                {
                    ret = handle_ssl_error( &sslerr, ssl, io_rc, sock, runex->conn_timeout);
                    if( sslerr != SSLACT_RETRY && sslerr != SSLACT_READ && sslerr != SSLACT_WRITE)
                    {
                        done = 1;
                        err = 1;
                        *rc = ret;
                        hold_err = ERR_peek_error();
                        if( hold_err)
                        {
                            fetch->end_errno = hold_err;
                            if( !fetch->err_msg) (void) stash_ssl_err_info( fetch, hold_err);
                        }
                        else
                        {
                            fetch->end_errno = errno;
                            fetch->err_msg = strdup( EMSG_SSL_CONN_FAIL);
                        }
		    }
		}
	    }
	}

#ifdef S2N_SUPPORT
        else if( targ->use_ssl && targ->use_s2n)
        {
            now = time( 0);
            deadline = now + (runex->conn_timeout / 1000);
            if( deadline <= now) deadline = now + 1;

            sock = fetch->conn_sock;
            s2n_connection_set_fd( fetch->s2n_conn, sock);
            s2n_connection_set_read_call( fetch->s2n_conn, s2n_raw_net_read);

            *rc = RC_NORMAL;
            for( pending = 1; pending && now <= deadline && *rc == RC_NORMAL; )
            {
                sysrc = s2n_negotiate( fetch->s2n_conn, &s2n_stat);

                if( !sysrc && s2n_stat == S2N_BLOCKED_ON_READ)
                {
                    sysrc = wait_until_sock_ready( sock, POLL_EVENTS_READ, runex->conn_timeout);
                    if( !sysrc) *rc = ERR_POLL_TIMEOUT;
                    else if( sysrc < 0) *rc = ERR_POLL_FAILED;
		}

                else if( !sysrc && s2n_stat == S2N_BLOCKED_ON_WRITE)
                {
                    sysrc = wait_until_sock_ready( sock, POLL_EVENTS_WRITE, runex->conn_timeout);
                    if( !sysrc) *rc = ERR_POLL_TIMEOUT;
                    else if( sysrc < 0) *rc = ERR_POLL_FAILED;
		}

                else
                {
                    pending = 0;
                    if( sysrc) *rc = ERR_SSL_NEGOTIATE;
		}

                now = time( 0);
	    }

            *rc = capture_checkpoint( fetch, EVENT_SSL_HANDSHAKE);
            if( *rc == RC_NORMAL) fetch->last_state |= LS_SSL_SHAKE_DONE;
	}
#endif

    }

    LEAVE( "ssl_handshake")

    return;
}

/* --- */

ssize_t s2n_raw_net_read(int fd, void *buffer, size_t blen)

{
    int io_rc, space, avail, refresh, ctype, sysrc;
    static struct fd_buffer_list *fd_list = 0;
    struct fd_buffer_list *walk = 0, *slot = 0, *before, *after;
    struct plan_data *plan;
    struct fetch_status *fetch;
    size_t rc = 0;

    /* --- */

    ENTER( "s2n_raw_net_read")

/* fprintf(stderr, "dbg:: raw-net-read: enter, fd=%d, len=%d\n", fd, (int) blen); */

    for( walk = fd_list; walk && !slot; )
    {
        if(walk->fd == fd) slot = walk;
        walk = walk->next;
    }

    if( !slot )
    {
        walk = (struct fd_buffer_list *) malloc(sizeof *walk);
        if( !walk ) ERR_EXIT("malloc failed")

        walk->fd = fd;
        walk->is_open = 1;
        walk->buff = (char *) malloc(READ_BUFF_SIZE);
        if( !walk->buff ) ERR_EXIT("malloc failed")
        walk->eod = walk->buff;
        walk->fence = walk->buff + READ_BUFF_SIZE;
        walk->pos = walk->buff;
        walk->prev = 0;
        walk->next = fd_list;

        if( fd_list) fd_list->prev = walk;
        fd_list = walk;

        slot = walk;
    }

    /* --- */

    avail = slot->eod - slot->pos;
    if( avail < blen && slot->is_open )
    {
        memmove(slot->buff, slot->pos, avail);
        slot->pos = slot->buff;
        slot->eod = slot->pos + avail;
        space = slot->fence - slot->eod;

        for( refresh = 0; !refresh; )
        {
            io_rc = read(fd, slot->eod, space);
/* fprintf(stderr, "dbg:: raw-net-read: read, rc=%d, max=%d, errno=%d\n", io_rc, space, (int) errno); */

            if( io_rc >= 0 )
            {
                if( !io_rc ) slot->is_open = 0;
                slot->eod = slot->eod + io_rc;
                refresh = 1;

                plan = register_current_plan(0);
                fetch = plan->status;
                if( fetch->last_state & LS_SSL_SHAKE_DONE ) ctype = EVENT_SSL_NET_READ;
                else ctype = EVENT_SSL_NEG_READ;
                sysrc = capture_checkpoint(fetch, ctype);
                if( sysrc == RC_NORMAL ) sysrc = add_datalen_block(fetch->lastcheck, io_rc);
	    }

            else if( errno != EINTR && errno != EAGAIN )
            {
                rc = -1;
                refresh = 1;
	    }
	}
    }

    /* --- */

    if( rc != -1 )
    {
        avail = slot->eod - slot->pos;
        if( blen <= avail ) io_rc = blen;
        else io_rc = avail;

        memcpy(buffer, slot->pos, io_rc);
        slot->pos += io_rc;
    }

    if( !slot->is_open && slot->pos >= slot->eod )
    {
        /* I/O channel is closed, and there's no data left in the buffer,
         * deallocate state tracking structure (and cache).
         */
        free(slot->buff);
        before = slot->prev;
        after = slot->next;
        if( before ) before->next = after;
        if( after ) after->prev = before;

        slot->fd = -1;
        slot->buff = 0;
        slot->prev = slot->next = 0;
        slot->eod = slot->fence = slot->pos = 0;
        free(slot);
    }

    /* --- */

/* fprintf(stderr, "dbg:: raw-net-read: leave, rc=%d, left=%ld, errno=%d\n", io_rc, slot->eod - slot->pos + 1, errno); */
    LEAVE( "s2n_raw_net_read")

    return(io_rc);
}
