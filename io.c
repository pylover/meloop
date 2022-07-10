#include "io.h"


#include <stdio.h>
#include <stdlib.h>


struct io {
    struct io_monad *monad;
    io_success success;
    io_failure fail;
};


struct io_monad {
    io_task run;
    void *args;
    struct io_monad *next;
};


static void _run(struct io *io, struct io_monad *m, void *data) {
    m->run(io, m->args, data);
}


void io_run(struct io_monad *m, void *data, io_success success, 
        io_failure fail) {
    struct io *io = malloc(sizeof(struct io));
    io->monad = m;
    io->success = success;
    io->fail = fail;
    _run(io, m, data);
}


void io_bind(struct io_monad *m1, struct io_monad *m2) {
    struct io_monad *last = m1;
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = m2;
}


void io_append(struct io_monad *m, io_task task, void* args) {
    io_bind(m, io_return(task, args));
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
    _run(io, next, result);
}


void io_pass(IO *io, void *args, void *data) {
    io_succeeded(io, data);
}


struct io_monad * io_return(io_task task, void *args) {
    struct io_monad *m = malloc(sizeof(struct io_monad));
    m->run = task;
    m->args = args;
    m->next = NULL;
    return m;
};


struct io_monad * io_new() {
    return io_return(io_pass, NULL); 
};


void io_free(struct io_monad *m) {
    if (m == NULL) {
        return;
    }
    io_free(m->next);
    free(m);
}
