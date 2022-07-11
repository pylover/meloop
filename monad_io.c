#include "monad_io.h"

#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
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
    void *data;
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


void mio_waitw (MonadContext *ctx, struct device *dev, void *data) {
    struct bag *bag = malloc(sizeof(bag));
    bag->ctx = ctx;
    bag->data = data;
    bag->dev = dev;

    if (_arm(dev->fd, EPOLLOUT, bag)) {
        monad_failed(ctx, "_arm");
        return;
    }
}


void mio_waitr (MonadContext *ctx, struct device *dev, void *data) {
    struct bag *bag = malloc(sizeof(bag));
    bag->ctx = ctx;
    bag->data = data;
    bag->dev = dev;

    if (_arm(dev->fd, EPOLLIN, bag)) {
        monad_failed(ctx, "_arm");
        return;
    }
}


int mio_run(struct monad *m, void *data, monad_success success, 
        monad_failure fail) {

    struct epoll_event events[MAX_EVENTS];
    struct epoll_event ev;
    struct bag *bag;
    int i;
    int nfds;
    MonadContext *ctx;
    struct device *dev;

    // TODO: run multiple monads, then wait
    monad_run(m, data, success, fail);
    
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
            data = bag->data;
            free(bag);

            if (ev.events & EPOLLRDHUP) {
                _dearm(dev->fd);
                monad_failed(ctx, "Remote hanged up");
            }
            else if (ev.events & EPOLLERR) {
                _dearm(dev->fd);
                monad_failed(ctx, "Connection Error");
            }
            else {
                monad_succeeded(ctx, data);
            }
        }
    }
    return OK;
}


void mio_init(int flags) {
    _epflags |= flags;
    _epfd = epoll_create1(0);
    if (_epfd < 0) {
        err(ERR, "epoll_create1");
    }
}


void mio_deinit() {
    close(_epfd);
}
