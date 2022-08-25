#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/ev.h"
#include "meloop/logging.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void 
waitA(struct circuitS *c, void *s, void *data, int fd, int op, int flags) {
    struct bagS *bag = meloop_bag_new(fd, c, s, data);
    
    if (meloop_ev_arm(op | flags, bag)) {
        ERROR_A(c, s, data, "meloop_ev_arm");
    }
}


void
writeA(struct circuitS *c, void *s, struct fileS *f, struct ioP *priv) {
    ssize_t size;
    
    size = write(f->fd, f->buffer, f->size);
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, f, f->fd, EPOLLOUT, priv->epollflags);
        }
        else {
            ERROR_A(c, s, f, "write");
        }
        return;
    }
    RETURN_A(c, s, f);
}


void
readA(struct circuitS *c, void *s, struct fileS *f, struct ioP *priv) {
    ssize_t size;

    /* Read from the file descriptor */
    size = read(f->fd, f->buffer, priv->readsize);

    /* Check for EOF */
    if (size == 0) {
        ERROR_A(c, s, f, "EOF");
        return;
    }

    /* Error | wait */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, f, f->fd, EPOLLIN, priv->epollflags);
        }
        else {
            ERROR_A(c, s, f, "read");
        }
        return;
    }
    f->size = size;
    RETURN_A(c, s, f);
}


void meloop_io_init(int flags) {
    meloop_ev_init(flags);
}


void meloop_io_deinit() {
    meloop_ev_deinit();
    meloop_bags_freeall();
}


/* Start event loop */
int 
meloop_io_loop(volatile int *status) {
    struct epoll_event events[EV_MAXEVENTS];
    struct epoll_event ev;
    struct bagS *bag;
    int i;
    int nfds;
    int fd;
    int ret = OK;
    struct circuitS *tmpcirc;
    void *tmpstate;
    void *tmpdata;

    while (((status == NULL) || (*status > EXIT_FAILURE)) && 
            meloop_ev_more()) {
        nfds = meloop_ev_wait(events);
        if (nfds < 0) {
            ret = ERR;
            break;
        }

        if (nfds == 0) {
            ret = OK;
            break;
        }
        
        
        for (i = 0; i < nfds; i++) {
            ev = events[i];
            bag = (struct bagS *) ev.data.ptr;
            tmpcirc = bag->circuit;
            tmpstate = bag->state;
            tmpdata = bag->data;
            fd = bag->fd;
            meloop_bag_free(bag);

            if (ev.events & EPOLLRDHUP) {
                meloop_ev_dearm(fd);
                close(fd);
                ERROR_A(tmpcirc, tmpstate, tmpdata, "Remote hanged up");
            }
            else if (ev.events & EPOLLERR) {
                meloop_ev_dearm(fd);
                close(fd);
                ERROR_A(tmpcirc, tmpstate, tmpdata, "Connection Error");
            }
            else {
                runA(tmpcirc, tmpstate, tmpdata);
            }
        }
    }

    return ret;
}
