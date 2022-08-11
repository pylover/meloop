#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/ev.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void 
waitA(struct circuitS *c, struct ioS *io, union any data, int fd, int op) {
    struct bagS *bag = meloop_bag_new(fd, c, io, data);
    
    if (meloop_ev_arm(op | io->epollflags, bag)) {
        perror("meloop_ev_arm"); 
        ERROR_A(c, io, data, "_arm");
    }
}


void
writeA(struct circuitS *c, struct ioS *io, struct stringS p) {
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
readA(struct circuitS *c, struct ioS *io, struct stringS p) {
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
    struct ioS *tmpio;
    union any tmpdata;

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
            tmpio = bag->io;
            tmpdata = bag->data;
            fd = bag->fd;
            meloop_bag_free(bag);

            if (ev.events & EPOLLRDHUP) {
                meloop_ev_dearm(fd);
                close(fd);
                ERROR_A(tmpcirc, tmpio, tmpdata, "Remote hanged up");
            }
            else if (ev.events & EPOLLERR) {
                meloop_ev_dearm(fd);
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
