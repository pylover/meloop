#include "monad/monad.h"
#include "monad/io.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <err.h>


#define CHUNK_SIZE  1024

#define ERROR -1
#define OK 0
static volatile int status = OK;


void promptM(MonadContext *ctx, struct device *dev, struct conn *c) {
    ssize_t size = write(c->wfd, ">>> ", 4);
    if (size < 4) {
        monad_failed(ctx, c, "write");
        return;
    }
    monad_succeeded(ctx, c);
}


void cleanitM(MonadContext *ctx, void *, struct conn *c) {
    c->size = 0;
    monad_succeeded(ctx, c);
}


void caseitM(MonadContext *ctx, void *, struct conn *c) {
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
    monad_io_init(0);

    struct conn c = {
        .rfd = STDIN_FILENO, 
        .wfd = STDOUT_FILENO, 
        .size = 0, 
        .data = malloc(CHUNK_SIZE), 
        .ptr = NULL
    };
    struct device dev = {false, CHUNK_SIZE};

    /* Draw circut */
    Monad *m = MONAD_RETURN(   awaitwM,  &dev );
               MONAD_APPEND(m, promptM,  &dev );
               MONAD_BIND  (m, readerF  (&dev));
               MONAD_APPEND(m, caseitM,  NULL );
               MONAD_BIND  (m, writerF  (&dev));
               MONAD_APPEND(m, cleanitM, NULL );

               // MONAD_APPEND(m, awaitrM,  &dev );
               // MONAD_APPEND(m, readerM,  &dev );
               // MONAD_APPEND(m, awaitwM,  &dev );
               // MONAD_APPEND(m, writerM,  &dev );

    /* Loop/Close it */
    monad_loop(m);

    if (MONAD_IO_RUN(m, &c, finish)) {
        err(1, "monad_io_run");
    }
   
    if (c.data != NULL) {
        free(c.data);
    }
    monad_free(m);
    monad_io_deinit();
    return status;
}
