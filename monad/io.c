#include "io.h"

#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
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


static int _nonblocking(int fd, bool enabled) {
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        return flags;
    }
    
    if (enabled) {
        flags |= O_NONBLOCK;
    }
    else {
        flags &= ~O_NONBLOCK;
    }

    return fcntl(fd, F_SETFL, flags);
}


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


void nonblockM(MonadContext *ctx, struct device *dev, struct conn *c) {
    int res;

    res = _nonblocking(c->rfd, true);
    if (res) {
        monad_failed(ctx, c, "enable nonblocking");
    }
    
    if (c->rfd != c->wfd) {
        res = _nonblocking(c->wfd, true);
        if (res) {
            monad_failed(ctx, c, "enable nonblocking");
        }
    }
    
    dev->nonblocking = true;
    monad_succeeded(ctx, c);
}


void blockM(MonadContext *ctx, struct device *dev, struct conn *c) {
    int res;

    res = _nonblocking(c->rfd, false);
    if (res) {
        monad_failed(ctx, c, "disable nonblocking");
    }
    
    if (c->rfd != c->wfd) {
        res = _nonblocking(c->wfd, false);
        if (res) {
            monad_failed(ctx, c, "disable nonblocking");
        }
    }
    
    dev->nonblocking = false;
    monad_succeeded(ctx, c);
}


void awaitwM(MonadContext *ctx, struct device *dev, struct conn *c) {
    monad_io_wait(ctx, dev, c, EPOLLOUT);
}


void awaitrM(MonadContext *ctx, struct device *dev, struct conn *c) {
    monad_io_wait(ctx, dev, c, EPOLLIN);
}


void readerM(MonadContext *ctx, struct device *dev, struct conn *c) {
    ssize_t size;

    /* Read from the file descriptor */
    size = read(c->rfd, c->data, dev->readsize);

    /* Check for EOF */
    if (size == 0) {
        monad_failed(ctx, c, "EOF");
        return;
    }
    
    /* Check for error */
    if (size < 0) {
        monad_failed(ctx, c, "read");
        return;
    }
    c->size = size;
    monad_succeeded(ctx, c);
}


void writerM(MonadContext *ctx, struct device *dev, struct conn *c) {
    /* Empty line */
    if (c->size == 1) {
        monad_succeeded(ctx, c);
        return;
    }

    ssize_t size = write(c->wfd, c->data, c->size);
    if (size < 0) {
        monad_failed(ctx, c, "write");
        return;
    }
    monad_succeeded(ctx, c);
}


struct monad * readerF(struct device *dev) {
    Monad *echo = MONAD_RETURN(      awaitrM, &dev);
                  MONAD_APPEND(echo, readerM, &dev);
   
    return echo;
}


struct monad * writerF(struct device *dev) {
    Monad *echo = MONAD_RETURN(      awaitwM, &dev);
                  MONAD_APPEND(echo, writerM, &dev);
   
    return echo;
}


struct monad * echoF(struct device *dev) {
    Monad *echo = readerF(dev);
    monad_bind(echo, writerF(dev));
    monad_loop(echo);
    return echo;
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
