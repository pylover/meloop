#include "io.h"


#include <stdio.h>
#include <stdlib.h>


struct io {
    struct io_monad *monad;
    io_success_callback success;
    io_fail_callback fail;
};


struct io_monad {
    io_task run;
    struct io_monad *next;
};


void io_run(struct io_monad *m, void *input, io_success_callback success,
        io_fail_callback fail) {
    // TODO: free
    struct io *io = malloc(sizeof(struct io));
    io->monad = m;
    io->success = success;
    io->fail = fail;
    m->run(io, input);
}


void io_bind(struct io_monad *m1, struct io_monad *m2) {
    struct io_monad *last = m1;
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = m2;
}


void io_append(struct io_monad *m1, io_task task) {
    io_bind(m1, io_new(task));
}


void io_failed(IO* io, const char *reason) {
    io->fail(reason);
}


void io_succeeded(IO* io, void *result) {
    struct io_monad *next = io->monad->next;
    if (next == NULL) {
        io->success(result);
        return;
    }

    io->monad = next;
    next->run(io, result);
}


IOMonad * io_new(io_task task) {
    // TODO: free
    struct io_monad *m = malloc(sizeof(struct io_monad));
    m->run = task;
    m->next = NULL;
    return m;
};
