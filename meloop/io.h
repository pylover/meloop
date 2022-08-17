#ifndef MELOOP_IO_H
#define MELOOP_IO_H


#include <meloop/arrow.h>

#include <stdlib.h>


struct stringS {
    char *buffer;
    size_t size;
};


struct fileS {
    struct stringS;
    int fd;
};


struct ioS {
    int epollflags;
    size_t readsize;
};


void 
waitA(struct circuitS *c, void *s, void *data, int fd, int op);


void
writeA(struct circuitS *c, void *s, struct fileS *f);


void
readA(struct circuitS *c, void *s, struct fileS *f);


void meloop_io_init(int flags);


void meloop_io_deinit();


int 
meloop_io_loop(volatile int *status);


/* Helper macros */
#define WAIT_A(c, s, d, f, op) \
    waitA(c, (struct fileS*)(s), (void*)(d), f, op);


#endif
