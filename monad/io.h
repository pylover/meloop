#ifndef MONAD_IO_H
#define MONAD_IO_H

#include "monad.h"

#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip.h>


struct io_props {
    int epollflags;
    int readsize;
};


struct conn {
    /* File descriptors */
    int rfd;
    int wfd;

    /* Buffer */
    size_t size;
    char *data;
    
    /* Address */
    struct sockaddr_in addr;

    /* user void ptr */
    void *ptr;

    /* Garbage for internal use, it should be freed by user. */
    void *garbage;
};


void 
monad_io_again(MonadContext *, struct io_props *, struct conn *c, int op);

int 
monad_io_loop(volatile int *status);

void 
monad_io_init(int flags);

void 
monad_io_deinit();


/* IO Monads */
void 
nonblockM(MonadContext *ctx, struct io_props *dev, struct conn *c);

void 
writerM(MonadContext *ctx, struct io_props *dev, struct conn *c);

void 
readerM(MonadContext *ctx, struct io_props *dev, struct conn *c);

struct monad * 
readerF(struct io_props *dev);

struct monad * 
writerF(struct io_props *dev); 

void 
printM(MonadContext *ctx, struct io_props *props, struct conn *c);


/* IO Monad Factories */
Monad * 
echoF(struct io_props *dev);

Monad * 
echoloopF(struct io_props *props);


#define MONAD_IO_AGAIN(c, p, d, op) \
    monad_io_again(c, (struct io_props *)p, (struct conn *)d, op);


#endif
