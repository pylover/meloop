#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <err.h>
#include <errno.h>


#define CHUNK_SIZE  1024

#define WORKING 9999
#define ERROR -1
#define OK 0
static volatile int status = WORKING;


struct packet {
    size_t size;
    char *data;
};


struct device {
    int fd;
};


void prompt (IO *io, struct device *dev, void *p) {
    ssize_t size = write(dev->fd, ">>> ", 4);
    if (size < 4) {
        io_failed(io, "write");
    }
    io_succeeded(io, p);
}


void readit (IO *io, struct device *dev, struct packet *p) {
    ssize_t size = read(dev->fd, p->data, CHUNK_SIZE);
    if (size <= 0) {
        io_failed(io, "getline");
        return;
    }
    p->size = size;
    io_succeeded(io, p);
}


void writeit (IO *io, struct device *dev, struct packet *p) {
    /* Empty line */
    if (p->size == 1) {
        io_succeeded(io, p);
        return;
    }

    ssize_t size = write(dev->fd, "... ", 4);
    if (size < 0) {
        io_failed(io, "write");
        return;
    }

    size = write(dev->fd, p->data, p->size);
    if (size < 0) {
        io_failed(io, "write");
        return;
    }
    io_succeeded(io, p);
}


void cleanit (IO *io, void *, struct packet *p) {
    p->size = 0;
    io_succeeded(io, p);
}


void caseit (IO *io, void *, struct packet *p) {
    char *s = p->data;
    size_t l = p->size;
    while (l) {
        *s = toupper((unsigned char) *s);
        s++;
        l--; 
    } 
    io_succeeded(io, p);
}


void failed(IO *io, const char *reason) {
    if (errno) {
        perror(reason);
        status = ERROR;
    }
    printf("\n");
    status = OK;
}


int main() {
    struct packet p = {0, malloc(CHUNK_SIZE)};
    struct device input = {STDIN_FILENO};
    struct device output = {STDOUT_FILENO};

    IOMonad *m = IO_RETURN(prompt, &output);
    IO_APPEND(m, readit, &input);
    IO_APPEND(m, caseit, NULL);
    IO_APPEND(m, writeit, &output);
    IO_APPEND(m, cleanit, NULL);
    
    while (status == WORKING) {
        io_run(m, &p, NULL, failed);
    }
   
    if (p.data != NULL) {
        free(p.data);
    }
    io_free(m);
    return status;
}
