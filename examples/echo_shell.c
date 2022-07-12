#include "monad/monad.h"
#include "monad/io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>


#define CHUNK_SIZE  1024

#define WORKING 9999
#define ERROR -1
#define OK 0
static volatile int status = WORKING;


void prompt(MonadContext *ctx, struct device *dev, struct conn *c) {
    ssize_t size = write(c->wfd, ">>> ", 4);
    if (size < 4) {
        monad_failed(ctx, c, "write");
        return;
    }
    monad_succeeded(ctx, c);
}


void cleanit(MonadContext *ctx, void *, struct conn *c) {
    c->size = 0;
    monad_succeeded(ctx, c);
}


void caseit(MonadContext *ctx, void *, struct conn *c) {
    char *s = c->data;
    size_t l = c->size;
    while (l) {
        *s = toupper((unsigned char) *s);
        s++;
        l--; 
    } 
    monad_succeeded(ctx, c);
}


void finish(MonadContext *ctx, struct conn *c, const char *reason) {
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
    
    printf("\n");
    status = OK;
}


int main() {
    mio_init(0);

    struct conn c = {
        .rfd = STDIN_FILENO, 
        .wfd = STDOUT_FILENO, 
        .size = 0, 
        .data = malloc(CHUNK_SIZE), 
        .ptr = NULL
    };
    struct device dev = {false, CHUNK_SIZE};

    /* Draw circut */
    Monad *m = MONAD_RETURN(   mio_waitw, &dev );
               MONAD_APPEND(m, prompt,    &dev );
               MONAD_APPEND(m, mio_waitr, &dev );
               MONAD_APPEND(m, mio_read,  &dev );
               MONAD_APPEND(m, caseit,    NULL );
               MONAD_APPEND(m, mio_waitw, &dev );
               MONAD_APPEND(m, mio_write, &dev );
               MONAD_APPEND(m, cleanit,   NULL );

    /* Loop/Close it */
    monad_loop(m);

    if (MIO_RUN(m, &c, finish)) {
        err(1, "mio_run");
    }
   
    if (c.data != NULL) {
        free(c.data);
    }
    monad_free(m);
    mio_deinit();
    return status;
}
