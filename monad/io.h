#ifndef MONAD_IO_H
#define MONAD_IO_H

#include "monad.h"

#include <stdio.h>
#include <stdbool.h>


struct device {
    bool nonblocking;
    int readsize;
};


struct conn {
    /* File descriptors */
    int rfd;
    int wfd;

    /* Buffer */
    size_t size;
    char *data;

    /* user void ptr */
    void *ptr;
};


void monad_io_wait(MonadContext *, struct device *, struct conn *c, int op);
int monad_io_run(struct monad *m, struct conn *conn, monad_finish);
void monad_io_init(int flags);
void monad_io_deinit();


/* IO Monads */
void awaitrM(MonadContext *ctx, struct device *dev, struct conn *c); 
void awaitwM(MonadContext *ctx, struct device *dev, struct conn *c);

void writerM(MonadContext *ctx, struct device *dev, struct conn *c);
void readerM(MonadContext *ctx, struct device *dev, struct conn *c);

struct monad * readerF(struct device *dev);
struct monad * writerF(struct device *dev); 


/* IO Monad Factories */
Monad * echoF(struct device *dev);


#define MONAD_IO_RUN(m, c, f) monad_io_run(m, c, (monad_finish)(f))


#endif
