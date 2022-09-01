#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/pipe.h"
#include "meloop/random.h"
#include "meloop/logging.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>


#define BUFFSIZE 32


struct ctx {
    struct fileS *rand;
    struct pipeS *pipe;
};


static void
_randA(struct circuitS *c, unsigned int *counter, struct ctx *ctx) {
    RETURN_A(c, counter, ctx->rand);
}


static void
_pipeA(struct circuitS *c, unsigned int *counter, struct ctx *ctx) {
    RETURN_A(c, counter, ctx->pipe);
}


static void
_ctxA(struct circuitS *c, unsigned int *counter, struct fileS *file) {
    RETURN_A(c, counter, file->ptr);
}


void
countA(struct circuitS *c, unsigned int *counter, struct pipeS *pipe) {
    (*counter)++;
    dprintf(pipe->wfd, "%.3u ", *counter);
    RETURN_A(c, counter, pipe);
}


void
errorcb(struct circuitS *c, unsigned int *counter, struct pipeS *d, 
        const char *e) {
    perror(e);
}


int main() {
    struct ctx ctx;
    unsigned int counter = 0;
    meloop_io_init(0);

    struct ioP io = {
        .readsize = BUFFSIZE,
        .epollflags = EPOLLET,
    };
    
    char buff[BUFFSIZE] = "\0";
    struct stringS data = {
        .blob = buff,
        .size = 0,
    };

    struct fileS rand = {
        .data = &data,
        .fd = -1,
        .ptr = &ctx,
    };
    struct pipeS pipe = {
        .data = &data,
        .rfd = STDIN_FILENO,
        .wfd = STDOUT_FILENO,
        .ptr = &ctx,
    };
    ctx.rand = &rand;
    ctx.pipe = &pipe;

    struct circuitS *c = NEW_C(errorcb);

                         APPEND_A(c, _randA,     NULL);
                         APPEND_A(c, randopenA,  &io );
    struct elementE *e = APPEND_A(c, randreadA,  &io );
                         APPEND_A(c, randencA,   &io );
                         APPEND_A(c, _ctxA,      NULL);
                         APPEND_A(c, _pipeA,     NULL);
                         APPEND_A(c, countA,     NULL);
                         APPEND_A(c, pipewriteA, &io );
                         APPEND_A(c, pipereadA,  &io );
                         APPEND_A(c, _ctxA,      NULL);
                         APPEND_A(c, _randA,     NULL);
               loopA(e);

    /* Run circuitS */
    // TODO: Use state for counter
    RUN_A(c, &counter, &ctx); 

    /* Start and wait for event loop */
    if (meloop_io_loop(NULL)) {
        err(1, "meloop_io_loop");
    }
    
    meloop_io_deinit();
    freeC(c);
    return EXIT_SUCCESS;
}
