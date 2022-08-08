#ifndef MELOOP_TIMER_H
#define MELOOP_TIMER_H


#include "meloop/io.h"


struct timerS {
    int fd;
    int clockid;
    int flags;
};


void
timeropenA(struct circuitS *c, struct ioS *io, struct stringS buff);


#endif
