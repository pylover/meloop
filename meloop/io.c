#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/ev.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void 
waitA(struct circuit *c, struct io *io, union any data, int fd, int op) {
    struct bag *bag = bag_new(c, io, data);
    
    if (ev_arm(fd, op | io->epollflags, bag)) {
        perror("ev_arm"); 
        ERROR_A(c, io, data, "_arm");
    }
}


void
writeA(struct circuit *c, struct io *io, struct string p) {
    ssize_t size;
    
    size = write(io->wfd, p.data, p.size);
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, io, p, io->wfd, EPOLLOUT);
        }
        else {
            ERROR_A(c, io, p, "write");
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
        ERROR_A(c, io, p, "EOF");
        return;
    }

    /* Error | wait */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, io, p, io->rfd, EPOLLIN);
        }
        else {
            ERROR_A(c, io, p, "read");
        }
        return;
    }
    p.size = size;
    RETURN_A(c, io, p);
}


void meloop_io_init(int flags) {
    ev_init(flags);
}


void meloop_io_deinit() {
    ev_deinit();
    bags_freeall();
}


/* Start event loop */
int 
meloop_io_loop(volatile int *status) {
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
                close(fd);
                ERROR_A(tmpcirc, tmpio, tmpdata, "Remote hanged up");
            }
            else if (ev.events & EPOLLERR) {
                ev_dearm(fd);
                close(fd);
                ERROR_A(tmpcirc, tmpio, tmpdata, "Connection Error");
            }
            else {
                runA(tmpcirc, tmpio, tmpdata);
            }
        }
    }

    return ret;
}
