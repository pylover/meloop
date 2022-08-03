#ifndef ARROW_IO_H
#define ARROW_IO_H


#include <arrow/types.h>
#include <arrow/arrow.h>

#include <stdlib.h>


struct conn {
    /* File descriptors */
    int rfd;
    int wfd;
        
    /* epoll */
    int epollflags;

    /* Behaviour */
    size_t readsize;
};


void 
waitA(struct circuit *c, struct conn *conn, union any data, int op);


void
writeA(struct circuit *c, struct conn *conn, struct string p);


void
readA(struct circuit *c, struct conn *conn, struct string p);


void arrow_io_init(int flags);


void arrow_io_deinit();


int 
arrow_io_loop(volatile int *status);


/* Helper macros */
#define WAIT_A(c, s, d, op) waitA(c, s, (union any)(d), op);


#endif
