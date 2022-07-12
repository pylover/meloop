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
void mio_waitr(MonadContext *ctx, struct device *dev, struct conn *c); 
void mio_waitw(MonadContext *ctx, struct device *dev, struct conn *c);
void mio_write(MonadContext *ctx, struct device *dev, struct conn *c);
void mio_read(MonadContext *ctx, struct device *dev, struct conn *c);

/* Monad Factories */
Monad * echoF(struct device *dev);

int mio_run(struct monad *m, struct conn *conn, monad_finish);
void mio_init(int flags);
void mio_deinit();


#define MIO_RUN(m, c, f) mio_run(m, c, (monad_finish)(f))


#endif
