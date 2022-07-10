#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <err.h>
#include <errno.h>


#define WORKING 9999
#define ERROR -1
#define OK 0
static volatile int status = WORKING;


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

    ssize_t size = write(STDOUT_FILENO, "... ", 4);
    if (size < 0) {
        io_failed(io, "write");
        return;
    }

    size = write(STDOUT_FILENO, p->data, p->size);
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


void caseit (IO *io, struct packet *p) {
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
    struct packet p = {0, NULL};

    IOMonad *m = IO_RETURN(prompt);
    IO_APPEND(m, readit);
    IO_APPEND(m, caseit);
    IO_APPEND(m, writeit);
    IO_APPEND(m, cleanit);
    
    while (status == WORKING) {
        io_run(m, &p, NULL, (io_task)failed);
    }
   
    io_free(m);
    return status;
}
