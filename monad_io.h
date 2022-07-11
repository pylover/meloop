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


void mio_waitr(MonadContext *ctx, struct device *dev, struct conn *c); 
void mio_waitw(MonadContext *ctx, struct device *dev, struct conn *c);
void mio_write(MonadContext *ctx, struct device *dev, struct conn *c);
void mio_read(MonadContext *ctx, struct device *dev, struct conn *c);

int mio_run(struct monad *m, struct conn *conn, monad_success, monad_failure);
void mio_init(int flags);
void mio_deinit();


#endif
