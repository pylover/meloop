// TODO: rename this module to file
#ifndef MELOOP_IO_H
#define MELOOP_IO_H


#include <meloop/arrow.h>

#include <stdlib.h>


struct stringS {
    char *blob;
    size_t size;
};


struct fileS {
    struct stringS *data;
    void *ptr;
    int fd;
};


struct ioP {
    int epollflags;
    size_t readsize;
};


void 
waitA(struct circuitS *c, void *s, void *data, int fd, int op, int flags);


void
writeA(struct circuitS *c, void *s, struct fileS *f, struct ioP *priv);


void
readA(struct circuitS *c, void *s, struct fileS *f, struct ioP *priv);


void 
meloop_io_init(int flags);


void 
meloop_io_deinit();


int 
meloop_io_loop(volatile int *status);


/* Helper macros */
#define WAIT_A(c, s, d, f, op, fl) \
    waitA(c, (struct fileS*)(s), (void*)(d), f, op, fl);


#endif
