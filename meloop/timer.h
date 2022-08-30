#ifndef MELOOP_TIMER_H
#define MELOOP_TIMER_H


#include "meloop/io.h"


struct timerP {
    int epollflags;
    int clockid;
    int flags;
    long interval_ns;
};


struct timerS {
    void *ptr;
    int fd;
    int status;
};


void
timeropenA(struct circuitS *c, void *global, struct timerS *t, 
        struct timerP *priv);


void 
timersleepA(struct circuitS *c, void *global, struct timerS *t,
        struct timerP *priv);


#endif
