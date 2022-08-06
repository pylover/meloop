#ifndef MELOOP_IO_H
#define MELOOP_IO_H


#include <meloop/types.h>
#include <meloop/arrow.h>

#include <stdlib.h>


struct ioS {
    /* epoll */
    int epollflags;

    /* Behaviour */
    size_t readsize;

    /* File descriptors */
    int rfd;
    int wfd;
};


void 
waitA(struct circuitS *c, struct ioS *io, union any data, int fd, int op);


void
writeA(struct circuitS *c, struct ioS *io, struct stringS p);


void
readA(struct circuitS *c, struct ioS *io, struct stringS p);


void meloop_io_init(int flags);


void meloop_io_deinit();


int 
meloop_io_loop(volatile int *status);


/* Helper macros */
#define WAIT_A(c, s, d, f, op) \
    waitA(c, (struct ioS*)(s), (union any)(d), f, op);


#endif
