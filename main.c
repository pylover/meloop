#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <err.h>


struct packet {
    size_t size;
    char *data;
};


void readit (IO *io, struct packet *p) {

    p->size = 0;
    p->data = NULL;
    // TODO: free
    ssize_t size = getline(&(p->data), &(p->size), stdin);
    if (size < 0) {
        io_failed(io, "getline");
        return;
    }
    p->size = size;
    io_succeeded(io, p);
}


void writeit (IO *io, struct packet *p) {
    ssize_t size = write(STDOUT_FILENO, p->data, p->size);
    if (size < 0) {
        io_failed(io, "write");
        return;
    }
    io_succeeded(io, p);
}


void ok(void *) {
    printf("Succeeded\n");
}


void failed(const char *reason) {
    printf("Failed: %s\n", reason);
}


int main() {
    IOMonad *m = io_new((io_task)readit);
    io_append(m, (io_task)writeit);
    
    struct packet p = {0, NULL};
    io_run(m, &p, ok, failed);
    return 0;
}
