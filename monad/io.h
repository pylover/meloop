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
};


void monad_again(MonadContext *, struct io_props *, struct conn *c, int op);
int monad_io_run(struct monad *m, struct conn *conn, monad_finish);
void monad_io_init(int flags);
void monad_io_deinit();


/* IO Monads */
void nonblockM(MonadContext *ctx, struct io_props *dev, struct conn *c);

void writerM(MonadContext *ctx, struct io_props *dev, struct conn *c);
void readerM(MonadContext *ctx, struct io_props *dev, struct conn *c);

struct monad * readerF(struct io_props *dev);
struct monad * writerF(struct io_props *dev); 


/* IO Monad Factories */
Monad * echoF(struct io_props *dev);


#define MONAD_IO_RUN(m, c, f) monad_io_run(m, c, (monad_finish)(f))


#endif
