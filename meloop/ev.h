#ifndef MELOOP_EV_H
#define MELOOP_EV_H


#include <sys/epoll.h>


#define EV_MAXEVENTS  16


/* A simple bag which used by waitA to hold meloop's essential data 
   until the underlying file descriptor becomes ready for read or write. */
struct bagS {
    int fd;
    struct circuitS *circuit;
    void *state;
    void *data;
};


void 
meloop_ev_init(int flags);


void 
meloop_ev_deinit();


int 
meloop_ev_arm(int op, struct bagS *bag);


int 
meloop_ev_dearm(int fd);


void
meloop_bag_free(struct bagS *bag);


void
meloop_bags_freeall();


struct bagS *
meloop_bag_new(int fd, struct circuitS *c, void *s, void *data);


int
meloop_ev_more();


int 
meloop_ev_wait(struct epoll_event *events);


#endif
