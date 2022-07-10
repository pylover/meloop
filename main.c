#include "monad.h"

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


void prompt (MonadContext *ctx, struct device *dev, void *p) {
    ssize_t size = write(dev->fd, ">>> ", 4);
    if (size < 4) {
        monad_failed(ctx, "write");
    }
    monad_succeeded(ctx, p);
}


void readit (MonadContext *ctx, struct device *dev, struct packet *p) {
    ssize_t size = read(dev->fd, p->data, CHUNK_SIZE);
    if (size <= 0) {
        monad_failed(ctx, "getline");
        return;
    }
    p->size = size;
    monad_succeeded(ctx, p);
}


void writeit (MonadContext *ctx, struct device *dev, struct packet *p) {
    /* Empty line */
    if (p->size == 1) {
        monad_succeeded(ctx, p);
        return;
    }

    ssize_t size = write(dev->fd, "... ", 4);
    if (size < 0) {
        monad_failed(ctx, "write");
        return;
    }

    size = write(dev->fd, p->data, p->size);
    if (size < 0) {
        monad_failed(ctx, "write");
        return;
    }
    monad_succeeded(ctx, p);
}


void cleanit (MonadContext *ctx, void *, struct packet *p) {
    p->size = 0;
    monad_succeeded(ctx, p);
}


void caseit (MonadContext *ctx, void *, struct packet *p) {
    char *s = p->data;
    size_t l = p->size;
    while (l) {
        *s = toupper((unsigned char) *s);
        s++;
        l--; 
    } 
    monad_succeeded(ctx, p);
}


void failed(MonadContext *ctx, const char *reason) {
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

    Monad *m = MONAD_RETURN(prompt, &output);
    MONAD_APPEND(m, readit, &input);
    MONAD_APPEND(m, caseit, NULL);
    MONAD_APPEND(m, writeit, &output);
    MONAD_APPEND(m, cleanit, NULL);
    
    while (status == WORKING) {
        monad_run(m, &p, NULL, failed);
    }
   
    if (p.data != NULL) {
        free(p.data);
    }
    monad_free(m);
    return status;
}
