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
    struct io_props *props;
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


void monad_again(MonadContext *ctx, struct io_props *props, 
        struct conn *c, int op) {
    struct bag *bag = malloc(sizeof(struct bag));
    if (bag == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }

    int fd = (op == EPOLLIN) ? c->rfd : c->wfd;
    // printf("bag_alloc: %d\n", fd);
    bag->ctx = ctx;
    bag->props = props;
    bag->conn = c;
    c->garbage = bag;
    
    if (_arm(fd, op | props->epollflags, bag)) {
        monad_failed(ctx, c, "_arm");
    }
}


void nonblockM(MonadContext *ctx, struct io_props *props, struct conn *c) {
    int res = _nonblocking(c->rfd, true);
    if (res) {
        monad_failed(ctx, c, "enable nonblocking");
        return;
    }
    
    if (c->rfd != c->wfd) {
        res = _nonblocking(c->wfd, true);
        if (res) {
            monad_failed(ctx, c, "enable nonblocking");
            return;
        }
    }
    monad_succeeded(ctx, c);
}


void readerM(MonadContext *ctx, struct io_props *props, struct conn *c) {
    ssize_t size;

    /* Read from the file descriptor */
    size = read(c->rfd, c->data, props->readsize);

    /* Check for EOF */
    if (size == 0) {
        monad_failed(ctx, c, "EOF");
        return;
    }
    
    /* Check for error */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            monad_again(ctx, props, c, EPOLLIN);
        }
        else {
            monad_failed(ctx, c, "read");
        }
        return;
    }
    c->size = size;
    monad_succeeded(ctx, c);
}


void writerM(MonadContext *ctx, struct io_props *props, struct conn *c) {
    ssize_t size;

    size = write(c->wfd, c->data, c->size);
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            monad_again(ctx, props, c, EPOLLIN);
        }
        else {
            monad_failed(ctx, c, "write");
        }
        return;
    }
    monad_succeeded(ctx, c);
}


struct monad * echoF(struct io_props *props) {
    Monad *echo = MONAD_RETURN(      readerM, props);
                  MONAD_APPEND(echo, writerM, props);
    
    monad_loop(echo);
    return echo;
}


struct monad * echoLoopF(struct io_props *props) {
    Monad *echo = echoF(props);
    monad_loop(echo);
    return echo;
}


int monad_io_run(struct monad *m, struct conn *conn, monad_finish finish,
        volatile int *status) {
    struct epoll_event events[MAX_EVENTS];
    struct epoll_event ev;
    struct bag *bag;
    int i;
    int nfds;
    int fd;
    int ret = OK;
    MonadContext *parent_context;
    MonadContext *ctx;
    struct io_props *props;

    /* trigger the monad */
    parent_context = monad_runall(m, conn, finish);
    
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
            ctx = bag->ctx;
            props = bag->props;
            conn = bag->conn;
            fd = (ev.events && EPOLLIN) ? conn->rfd : conn->wfd;
            // printf("bag_free: %d\n", fd);
            free(bag);
            conn->garbage = NULL;

            if (ev.events & EPOLLRDHUP) {
                _dearm(fd);
                monad_failed(ctx, conn, NULL);
            }
            else if (ev.events & EPOLLERR) {
                _dearm(fd);
                monad_failed(ctx, conn, "Connection Error");
            }
            else {
                monad_execute(ctx, conn);
            }
        }
    }
    if (parent_context != NULL) {
        monad_terminate(parent_context, conn, NULL);
    }
    return ret;
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
