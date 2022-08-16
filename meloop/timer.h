#ifndef MELOOP_TIMER_H
#define MELOOP_TIMER_H


#include "meloop/io.h"


struct timerS {
    int fd;
    int clockid;
    int flags;
    int status;
    long interval_ns;
};


void
timeropenA(struct circuitS *c, struct fileS *io, struct stringS buff);


void 
timersleepA(struct circuitS *c, struct fileS *s, struct stringS d);


#endif
