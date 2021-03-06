#include "monad/monad.h"
#include "monad/io.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <err.h>
#include <sys/epoll.h>


#define CHUNK_SIZE  1024

#define WORKING 9999
#define ERROR -1
#define OK 0
static volatile int status = WORKING;


void promptM(MonadContext *ctx, struct io_props *dev, struct conn *c) {
    c->size = sprintf(c->data, ">>> ");
    writerM(ctx, dev, c);
}


void resetM(MonadContext *ctx, void *, struct conn *c) {
    c->size = 0;
    memset(c->data, 0, CHUNK_SIZE);
    monad_succeeded(ctx, c);
}


void caseitM(MonadContext *ctx, void *, struct conn *c) {
    char *s = c->data;
   
    /* Ignore empty line */
    if (strlen(s) == 1) {
        s[0] = 0;
    }

    size_t l = c->size;
    while (l) {
        *s = toupper((unsigned char) *s);
        s++;
        l--; 
    } 
    monad_succeeded(ctx, c);
}


void finish(MonadContext *ctx, struct conn *c, const char *reason) {
    if (reason != NULL) {
        /* CTRL+D */
        if (strstr(reason, "EOF")) {
            printf("%s\n", reason);
            status = OK;
            return;
        }
        else {
            perror(reason);
            status = ERROR;
        }
    }
    
    printf("\n");
    status = OK;
}


int main() {
    monad_io_init(0);

    struct conn c = {
        .rfd = STDIN_FILENO, 
        .wfd = STDOUT_FILENO, 
        .size = 0, 
        .data = malloc(CHUNK_SIZE), 
        .ptr = NULL
    };
    struct io_props dev = {EPOLLET, CHUNK_SIZE};

    /* Draw circut */
    Monad *init = MONAD_RETURN(nonblockM, &dev);

    Monad *loop = MONAD_RETURN(      promptM, &dev);
                  MONAD_APPEND(loop, resetM,  NULL);
                  MONAD_APPEND(loop, readerM, &dev);
                  MONAD_APPEND(loop, caseitM, NULL);
                  MONAD_APPEND(loop, writerM, &dev);
                  MONAD_APPEND(loop, resetM,  NULL);

    /* Loop/Close it */
    monad_loop(loop);
    monad_bind(init, loop);
   
    /* Run echo shell monad */
    MONAD_RUN(init, &c, finish);
    
    /* Start and wait for event loop */
    if (monad_io_loop(&status)) {
        err(1, "monad_io_run");
    }
    
    /* Dispose */
    if (c.data != NULL) {
        free(c.data);
    }
    monad_free(init);
    monad_io_deinit();
    return status;
}
