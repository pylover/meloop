#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <err.h>
#include <errno.h>


struct packet {
    size_t size;
    char *data;
};


void prompt (IO *io, void *p) {
    printf(">>> ");
    io_succeeded(io, p);
}


void readit (IO *io, struct packet *p) {
    ssize_t size = getline(&(p->data), &(p->size), stdin);
    if (size <= 0) {
        io_failed(io, "getline");
        return;
    }
    p->size = size;
    io_succeeded(io, p);
}


void writeit (IO *io, struct packet *p) {
    /* Empty line */
    if (p->size == 1) {
        io_succeeded(io, p);
        return;
    }
    ssize_t size = write(STDOUT_FILENO, p->data, p->size);
    if (size < 0) {
        io_failed(io, "write");
        return;
    }
    io_succeeded(io, p);
}


void cleanit (IO *io, struct packet *p) {
    p->size = 0;
    if (p->data != NULL) {
        free(p->data);
    }
    p->data = NULL;
    io_succeeded(io, p);
}


void ok(IO *io, void *) {
    printf("Succeeded\n");
}


void failed(IO *io, const char *reason) {
    if (errno) {
        err(1, "Failed: %s\n", reason);
    }
    printf("\n");
    exit(0);
}


int main() {
    struct packet p = {0, NULL};

    IOMonad *m = io_new();
    io_append(m, (io_task) prompt);
    io_append(m, (io_task) readit);
    io_append(m, (io_task) writeit);
    io_append(m, (io_task) cleanit);
    
    while (true) {
        io_run(m, &p, NULL, (io_task)failed);
    }
    return 0;
}
