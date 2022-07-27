#include <arrow/arrow.h>
#include <arrow/ev.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>


static int _epfd = -1;
static int _epflags = EPOLLONESHOT | EPOLLRDHUP | EPOLLERR;
static volatile int _waitfds = 0;


#define MAX_EVENTS  16


void 
ev_init(int flags) {
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


void 
ev_deinit() {
    close(_epfd);
}


int 
ev_arm(int fd, int op, struct bag *bag) {
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


int 
ev_dearm(int fd) {
    if (epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL) != OK) {
        return ERR;
    }
    return OK;
}
