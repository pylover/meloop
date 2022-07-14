#ifndef MONAD_IO_H
#define MONAD_IO_H

#include "monad.h"

#include <stdio.h>
#include <stdbool.h>


struct device {
    bool nonblock;
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


/* Monads */
void monad_io_waitrM(MonadContext *ctx, struct device *dev, struct conn *c); 
void monad_io_waitwM(MonadContext *ctx, struct device *dev, struct conn *c);
void monad_io_writeM(MonadContext *ctx, struct device *dev, struct conn *c);
void monad_io_readM(MonadContext *ctx, struct device *dev, struct conn *c);

/* Monad Factories */
Monad * echoF(struct device *dev);

int monad_io_run(struct monad *m, struct conn *conn, monad_finish);
void monad_io_init(int flags);
void monad_io_deinit();


#define MONAD_IO_RUN(m, c, f) monad_io_run(m, c, (monad_finish)(f))


#endif
