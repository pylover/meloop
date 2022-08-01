#ifndef ARROW_IO_H
#define ARROW_IO_H


struct conn {
    /* File descriptors */
    int rfd;
    int wfd;
    
    /* epoll */
    int epollflags;
};



void 
waitA(struct circuit *c, struct conn *conn, union any data, int op);


void
writeA(struct circuit *c, struct conn *conn, struct string p);


void arrow_io_init(int flags);


void arrow_io_deinit();


/* Helper macros */
#define WAIT_A(c, s, d, op) waitA(c, s, (union any)(d), op);


#endif
