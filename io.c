#include "io.h"


#include <stdio.h>
#include <stdlib.h>


struct io {
    struct io_monad *monad;
    io_task success;
    io_task fail;
};


struct io_monad {
    io_task run;
    struct io_monad *next;
};


void io_run(struct io_monad *m, void *input, io_task success, io_task fail) {
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


void io_append(struct io_monad *m, io_task task) {
    io_bind(m, io_return(task));
}


void io_failed(IO* io, const char *reason) {
    if (io->fail != NULL) {
        io->fail(io, (char *)reason);
    }
    free(io);
}


void io_succeeded(IO* io, void *result) {
    struct io_monad *next = io->monad->next;
    if (next == NULL) {
        if (io->success != NULL) {
            io->success(io, result);
        }
        free(io);
        return;
    }

    io->monad = next;
    next->run(io, result);
}


void io_pass(IO *io, void *a) {
    io_succeeded(io, a);
}


struct io_monad * io_return(io_task task) {
    struct io_monad *m = malloc(sizeof(struct io_monad));
    m->run = task;
    m->next = NULL;
    return m;
};


struct io_monad * io_new() {
    return io_return(io_pass); 
};


void io_free(struct io_monad *m) {
    if (m == NULL) {
        return;
    }
    io_free(m->next);
    free(m);
}
