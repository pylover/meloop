#include "arrow/arrow.h"
#include "arrow/io.h"
#include "arrow/ev.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void 
waitA(struct circuit *c, struct io *io, union any data, int fd, int op) {
    struct bag *bag = bag_new(c, io, data);
    
    //printf("IO wait\n");
    if (ev_arm(fd, op | io->epollflags, bag)) {
        errorA(c, io, "_arm");
    }
}


void
writeA(struct circuit *c, struct io *io, struct string p) {
    ssize_t size;
    
    size = write(io->wfd, p.data, p.size);
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, io, p, EPOLLOUT, io->wfd);
        }
        else {
            errorA(c, io, "write");
        }
        return;
    }
    RETURN_A(c, io, p);
}


void
readA(struct circuit *c, struct io *io, struct string p) {
    ssize_t size;
    
    /* Read from the file descriptor */
    size = read(io->rfd, p.data, io->readsize);

    /* Check for EOF */
    if (size == 0) {
        errorA(c, io, "EOF");
        return;
    }

    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, io, p, io->rfd, EPOLLIN);
        }
        else {
            errorA(c, io, "read");
        }
        return;
    }
    p.size = size;
    RETURN_A(c, io, p);
}


void arrow_io_init(int flags) {
    ev_init(flags);
}


void arrow_io_deinit() {
    ev_deinit();
    bags_freeall();
}


/* Start event loop */
int 
arrow_io_loop(volatile int *status) {
    struct epoll_event events[EV_MAXEVENTS];
    struct epoll_event ev;
    struct bag *bag;
    int i;
    int nfds;
    int fd;
    int ret = OK;
    struct circuit *tmpcirc;
    struct io *tmpio;
    union any tmpdata;

    while (((status == NULL) || (*status > EXIT_FAILURE)) && ev_more()) {
        nfds = ev_wait(events);
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
            bag = (struct bag *) ev.data.ptr;
            tmpcirc = bag->circuit;
            tmpio = bag->io;
            tmpdata = bag->data;
            fd = (ev.events && EPOLLIN) ? tmpio->rfd : tmpio->wfd;
            bag_free(bag);

            if (ev.events & EPOLLRDHUP) {
                ev_dearm(fd);
                RETURN_A(tmpcirc, tmpio, tmpdata);
            }
            else if (ev.events & EPOLLERR) {
                ev_dearm(fd);
                errorA(tmpcirc, tmpio, "Connection Error");
            }
            else {
                runA(tmpcirc, tmpio, tmpdata);
            }
        }
    }

    return ret;
}
