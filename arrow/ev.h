#ifndef ARROW_EV_H
#define ARROW_EV_H


#include "arrow/types.h"

#include <sys/epoll.h>


#define EV_MAXEVENTS  16


/* A simple bag which used by waitA to hold arrow's essential data 
   until the underlying file descriptor becomes ready for read or write. */
struct bag {
    struct circuit *circuit;
    struct io *io;
    union any data;
};


void 
ev_init(int flags);


void 
ev_deinit();


int 
ev_arm(int fd, int op, struct bag *bag);


int 
ev_dearm(int fd);


void
bag_free(struct bag *bag);


void
bags_freeall();


struct bag *
bag_new(struct circuit *c, struct io *io, union any data);


int
ev_more();


int 
ev_wait(struct epoll_event *events);


#endif
