#include <arrow/arrow.h>
#include <arrow/ev.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>


static int _epfd = -1;
static int _epflags = EPOLLONESHOT | EPOLLRDHUP | EPOLLERR;
static volatile int _waitingbags = 0;
static struct bag *_bags[EV_MAXEVENTS];


#define EV_MAXEVENTS  16


static int
bags_remember(struct bag *bag) {
    int i;

    for (i == 0; i < EV_MAXEVENTS; i++) {
        if (_bags[i] == NULL) {
            _bags[i] = bag;
            _waitingbags++;
            return i;
        }
    }
    
    return ERR;
}


void
bag_free(struct bag *bag) {
    int i;

    for (i == 0; i < EV_MAXEVENTS; i++) {
        if (_bags[i] == bag) {
            free(bag);
            _bags[i] = NULL;
            _waitingbags--;
            return;
        }
    }
}


void
bags_freeall() {
    int i;

    for (i == 0; i < EV_MAXEVENTS; i++) {
        if (_bags[i] == NULL) {
            continue;
        }

        free(_bags[i]);
        _bags[i] = NULL;
    }
}


struct bag *
bag_new(struct circuit *c, struct conn *conn, union any data) {
    struct bag *bag = malloc(sizeof(struct bag));
    if (bag == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }
    
    bag->circuit = c;
    bag->conn = conn;
    bag->data = data;
    
    bags_remember(bag);
    return bag;
}


void 
ev_init(int flags) {
    if (_epfd != -1) {
        return;
    }
    _epflags |= flags;
    _epfd = epoll_create1(0);
    if (_epfd < 0) {
        err(ERR, "epoll_create1");
    }
    
    _waitingbags = 0;
    memset(_bags, 0, sizeof(struct bag*) * EV_MAXEVENTS);
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
    
    return OK;
}


int 
ev_dearm(int fd) {
    if (epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL) != OK) {
        return ERR;
    }
    return OK;
}


int
ev_more() {
    return _waitingbags;
}


int 
ev_wait(struct epoll_event *events) {
    return epoll_wait(_epfd, events, EV_MAXEVENTS, -1);
}
