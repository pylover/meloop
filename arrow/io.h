#ifndef ARROW_IO_H
#define ARROW_IO_H


struct conn {
    /* File descriptors */
    int rfd;
    int wfd;
    
    /* epoll */
    int epollflags;
};



#define WAIT_A(c, s, d, op) waitA(c, s, (union args)(p), op);


void 
waitA(struct circuit *c, struct conn *conn, union args data, int op);


void
writeA(struct circuit *c, struct conn *conn, struct string p);


void arrow_io_init(int flags);


void arrow_io_deinit();


#endif
