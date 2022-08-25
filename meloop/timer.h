#ifndef MELOOP_TIMER_H
#define MELOOP_TIMER_H


#include "meloop/io.h"


struct timerP {
    int epollflags;
    int fd;
    int clockid;
    int flags;
    int status;
    long interval_ns;
};


void
timeropenA(struct circuitS *c, void *global, void *data, 
        struct timerP *priv);


void 
timersleepA(struct circuitS *c, void *global, void *data,
        struct timerP *priv);


#endif
