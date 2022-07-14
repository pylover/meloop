#include "io.h"

#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/epoll.h>


static int _epfd = -1;
static int _epflags = EPOLLONESHOT | EPOLLRDHUP | EPOLLERR;
static volatile int _waitfds = 0;


#define MAX_EVENTS  16
#define ERR -1
#define OK 0


struct bag {
    MonadContext *ctx;
    struct device *dev;
    struct conn *conn;
};


static int _arm(int fd, int op, struct bag *bag) {
    struct epoll_event ev;
    ev.events = _epflags | op;
    ev.data.ptr = bag;

    if (epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev) != OK) {
        if (errno == ENOENT) {
            /* File descriptor is not exists yet */
            if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev) != OK) {
                return ERR;
            }
        }
        else {
            return ERR;
        }
    }
    
    _waitfds++;
    return OK;
}


static int _dearm(int fd) {
    if (epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL) != OK) {
        return ERR;
    }
    return OK;
}


void monad_io_wait(MonadContext *ctx, struct device *dev, 
        struct conn *c, int op) {
    struct bag *bag = malloc(sizeof(bag));
    int fd = (op == EPOLLIN) ? c->rfd : c->wfd;
    bag->ctx = ctx;
    bag->conn = c;
    bag->dev = dev;

    if (_arm(fd, op, bag)) {
        monad_failed(ctx, c, "_arm");
        return;
    }
}


int monad_io_run(struct monad *m, struct conn *conn, monad_finish finish) {
    struct epoll_event events[MAX_EVENTS];
    struct epoll_event ev;
    struct bag *bag;
    int i;
    int nfds;
    int fd;
    MonadContext *ctx;
    struct device *dev;

    // TODO: run multiple monads, then wait
    monad_run(m, conn, finish);
    
    while (_waitfds) {
        nfds = epoll_wait(_epfd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            return ERR;
        }

        if (nfds == 0) {
            return OK;
        }
        
        for (i = 0; i < nfds; i++) {
            _waitfds--;
            ev = events[i];
            bag = (struct bag *) ev.data.ptr;
            ctx = bag->ctx;
            dev = bag->dev;
            conn = bag->conn;
            fd = (ev.events && EPOLLIN) ? conn->rfd : conn->wfd;
            free(bag);

            if (ev.events & EPOLLRDHUP) {
                _dearm(fd);
                monad_failed(ctx, conn, "Remote hanged up");
            }
            else if (ev.events & EPOLLERR) {
                _dearm(fd);
                monad_failed(ctx, conn, "Connection Error");
            }
            else {
                monad_succeeded(ctx, conn);
            }
        }
    }
    return OK;
}


void monad_io_init(int flags) {
    if (_epfd != -1) {
        return;
    }
    _waitfds = 0;
    _epflags |= flags;
    _epfd = epoll_create1(0);
    if (_epfd < 0) {
        err(ERR, "epoll_create1");
    }
}


void monad_io_deinit() {
    close(_epfd);
}
