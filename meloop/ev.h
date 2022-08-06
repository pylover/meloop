#ifndef MELOOP_EV_H
#define MELOOP_EV_H


#include "meloop/types.h"

#include <sys/epoll.h>


#define EV_MAXEVENTS  16


/* A simple bag which used by waitA to hold meloop's essential data 
   until the underlying file descriptor becomes ready for read or write. */
struct bag {
    struct circuit *circuit;
    struct io *io;
    union any data;
};


void 
meloop_ev_init(int flags);


void 
meloop_ev_deinit();


int 
meloop_ev_arm(int fd, int op, struct bag *bag);


int 
meloop_ev_dearm(int fd);


void
meloop_bag_free(struct bag *bag);


void
meloop_bags_freeall();


struct bag *
meloop_bag_new(struct circuit *c, struct io *io, union any data);


int
meloop_ev_more();


int 
meloop_ev_wait(struct epoll_event *events);


#endif
