#ifndef MELOOP_IO_H
#define MELOOP_IO_H


#include <meloop/types.h>
#include <meloop/arrow.h>

#include <stdlib.h>


struct ioS {
    int epollflags;
    size_t readsize;
};


void 
waitA(struct circuitS *c, void *s, union any data, int fd, int op);


void
writeA(struct circuitS *c, void *s, struct fileS f);


void
readA(struct circuitS *c, void *s, struct fileS f);


void meloop_io_init(int flags);


void meloop_io_deinit();


int 
meloop_io_loop(volatile int *status);


/* Helper macros */
#define WAIT_A(c, s, d, f, op) \
    waitA(c, (struct fileS*)(s), (union any)(d), f, op);


#endif
