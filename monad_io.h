#ifndef MONAD_IO_H
#define MONAD_IO_H

#include "monad.h"

#include <stdio.h>


struct device {
    int fd;
};


struct packet {
    size_t size;
    char *data;
};


void mio_waitforwrite (MonadContext *ctx, struct device *, void *);
int mio_run(struct monad *m, void *data, monad_success, monad_failure);
void mio_init(int flags);
void mio_deinit();


#endif
