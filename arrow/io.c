#include "arrow/arrow.h"
#include "arrow/io.h"
#include "arrow/ev.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>


static int _epfd = -1;
static int _epflags = EPOLLONESHOT | EPOLLRDHUP | EPOLLERR;
static volatile int _waitfds = 0;


#define MAX_EVENTS  16


void 
waitA(struct circuit *c, struct conn *conn, union any data, int op) {
    struct bag *bag = malloc(sizeof(struct bag));
    if (bag == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }
    
    printf("IO wait\n");
    int fd = (op == EPOLLIN) ? conn->rfd : conn->wfd;
    bag->circuit = c;
    bag->conn = conn;
    bag->data = data;
    
    if (ev_arm(fd, op | conn->epollflags, bag)) {
        errorA(c, conn, "_arm");
    }
}


void
writeA(struct circuit *c, struct conn *conn, struct string p) {
    ssize_t size;
    
    size = write(conn->wfd, p.data, p.size);
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, conn, p, EPOLLIN);
        }
        else {
            errorA(c, conn, "write");
        }
        return;
    }
    RETURN_A(c, conn, p);
}


void
readA(struct circuit *c, struct conn *conn, struct string p) {
    ssize_t size;
    
    /* Read from the file descriptor */
    size = read(conn->rfd, p.data, conn->readsize);

    /* Check for EOF */
    if (size == 0) {
        errno = ECANCELED;
        errorA(c, conn, "EOF");
        return;
    }

    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, conn, p, EPOLLIN);
        }
        else {
            errorA(c, conn, "read");
        }
        return;
    }
    p.size = size;
    RETURN_A(c, conn, p);
}


void arrow_io_init(int flags) {
    ev_init(flags);
}


void arrow_io_deinit() {
    ev_deinit();
}


/* Start event loop */
int 
arrow_io_loop(volatile int *status) {
    struct epoll_event events[MAX_EVENTS];
    struct epoll_event ev;
    struct bag *bag;
    struct conn *conn;
    int i;
    int nfds;
    int fd;
    int ret = OK;
    struct circuit *tmpcirc;
    struct conn *tmpconn;
    union any tmpdata;

    while (((status == NULL) || (*status > EXIT_FAILURE)) && _waitfds) {
        nfds = epoll_wait(_epfd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            ret = ERR;
            break;
        }

        if (nfds == 0) {
            ret = OK;
            break;
        }
        
        for (i = 0; i < nfds; i++) {
            _waitfds--;
            ev = events[i];
            bag = (struct bag *) ev.data.ptr;
            tmpcirc = bag->circuit;
            tmpconn = bag->conn;
            tmpdata = bag->data;
            fd = (ev.events && EPOLLIN) ? conn->rfd : conn->wfd;
            free(bag);

            if (ev.events & EPOLLRDHUP) {
                ev_dearm(fd);
                RETURN_A(tmpcirc, tmpconn, tmpdata);
            }
            else if (ev.events & EPOLLERR) {
                ev_dearm(fd);
                errorA(tmpcirc, tmpconn, "Connection Error");
            }
            else {
                runA(tmpcirc, conn, tmpdata);
            }
        }
    }

    return ret;
}
