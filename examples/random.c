#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/pipe.h"
#include "meloop/random.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>


#define BUFFSIZE 32


void
countA(struct circuitS *c, unsigned int *counter, struct pipeS *pipe) {
    (*counter)++;
    dprintf(pipe->wfd, "%.3u ", *counter);
    RETURN_A(c, counter, pipe);
}


void
errorcb(struct circuitS *c, struct fileS *io, struct stringS d, const char *e) {
    perror(e);
}


void
successcb(struct circuitS *c, struct fileS *io, int out) {
    printf("Out: %d\n", out);
}


int main() {
    unsigned int counter = 0;
    meloop_io_init(0);

    struct randS rand = {
        .readsize = BUFFSIZE,
        .epollflags = EPOLLET,
        .fd = -1,
    };
    
    char buff[BUFFSIZE] = "\0";
    struct pipeS pipe = {
        .buffer = buff,
        .size = 0,
        .rfd = STDIN_FILENO,
        .wfd = STDOUT_FILENO,
    };

    struct circuitS *c = NEW_C(successcb, errorcb);

                         APPEND_A(c, randopenA,  &rand);
    struct elementE *e = APPEND_A(c, randreadA,  &rand);
                         APPEND_A(c, randencA,   &rand);
                         APPEND_A(c, countA,     NULL);
                         APPEND_A(c, pipewriteA, &rand);
                         APPEND_A(c, pipereadA,  &rand);
               loopA(e);

    /* Run circuitS */
    // TODO: Use state for counter
    RUN_A(c, &counter, &pipe); 

    /* Start and wait for event loop */
    if (meloop_io_loop(NULL)) {
        err(1, "meloop_io_loop");
    }
    
    meloop_io_deinit();
    freeC(c);
    return EXIT_SUCCESS;
}
