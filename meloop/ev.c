#include <meloop/arrow.h>
#include <meloop/ev.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>


static int _epfd = -1;
static int _epflags = EPOLLONESHOT | EPOLLRDHUP | EPOLLERR;
static volatile int _waitingbags = 0;
static struct bagS *_bags[EV_MAXEVENTS];


#define EV_MAXEVENTS  16


static int
bags_remember(struct bagS *bag) {
    int i;

    for (i = 0; i < EV_MAXEVENTS; i++) {
        if (_bags[i] == NULL) {
            _bags[i] = bag;
            _waitingbags++;
            return i;
        }
    }
    
    return ERR;
}


void
meloop_bag_free(struct bagS *bag) {
    int i;

    for (i = 0; i < EV_MAXEVENTS; i++) {
        if (_bags[i] == bag) {
            free(bag);
            _bags[i] = NULL;
            _waitingbags--;
            return;
        }
    }
}


void
meloop_bags_freeall() {
    struct bagS *bag;
    int i;

    for (i = 0; i < EV_MAXEVENTS; i++) {
        bag = _bags[i];

        if (bag == NULL) {
            continue;
        }

        ERROR_A(bag->circuit, bag->io, bag->data, "bag_free");
        free(bag);
        _bags[i] = NULL;
    }
}


struct bagS *
meloop_bag_new(struct circuitS *c, struct ioS *io, union any data) {
    struct bagS *bag = malloc(sizeof(struct bagS));
    if (bag == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }
    
    bag->circuit = c;
    bag->io = io;
    bag->data = data;
    
    bags_remember(bag);
    return bag;
}


void 
meloop_ev_init(int flags) {
    if (_epfd != -1) {
        return;
    }
    _epflags |= flags;
    _epfd = epoll_create1(0);
    if (_epfd < 0) {
        err(ERR, "epoll_create1");
    }
    
    _waitingbags = 0;
    memset(_bags, 0, sizeof(struct bagS*) * EV_MAXEVENTS);
}


void 
meloop_ev_deinit() {
    close(_epfd);
}


int 
meloop_ev_arm(int fd, int op, struct bagS *bag) {
    struct epoll_event ev;
    
    ev.events = _epflags | op;
    ev.data.ptr = bag;
    
    if (epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev) != OK) {
        if (errno == ENOENT) {
            errno = 0;
            /* File descriptor is not exists yet */
            if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev) != OK) {
                return ERR;
            }
        }
        else {
            return ERR;
        }
    }
    
    return OK;
}


int 
meloop_ev_dearm(int fd) {
    if (epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL) != OK) {
        return ERR;
    }
    return OK;
}


int
meloop_ev_more() {
    return _waitingbags;
}


int 
meloop_ev_wait(struct epoll_event *events) {
    return epoll_wait(_epfd, events, EV_MAXEVENTS, -1);
}
