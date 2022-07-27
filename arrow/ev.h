#ifndef ARROW_EV_H
#define ARROW_EV_H


/* A simple bag which used by waitA to hold arrow's essential data 
   until the underlying file descriptor becomes ready for read or write. */
struct bag {
    struct circuit *circuit;
    struct conn *conn;
    union args data;
};


void 
ev_init(int flags);


void 
ev_deinit();


int 
ev_arm(int fd, int op, struct bag *bag);


int 
ev_dearm(int fd);


#endif
