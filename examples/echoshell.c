#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/pipe.h"
#include "meloop/logging.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>


#define BUFFSIZE 1024


void 
promptA(struct circuitS *c, unsigned int *counter, struct pipeS *pipe, 
        const char *priv) {
    (*counter)++;
    dprintf(pipe->wfd, "%.3u %s", *counter, priv);
    RETURN_A(c, counter, pipe);
}


void 
echoA(struct circuitS *c, unsigned int *counter, struct pipeS *pipe) {
    if ((pipe->size == 1) && (pipe->buffer[0] == '\n')) {
        pipe->size = 0;
    }
    RETURN_A(c, counter, pipe);
}


void
errorcb(struct circuitS *c, unsigned int *counter, void *data, 
        const char *error) {
    ERROR(error);
}


int main() {
    unsigned int counter = 0;
    logging_verbosity = LOGGING_DEBUG;
    meloop_io_init(0);

    struct ioP io = {
        .readsize = BUFFSIZE,
        .epollflags = EPOLLET,
    };

    char buff[BUFFSIZE] = "\0";
    struct pipeS pipe = {
        .buffer = buff,
        .size = 0,
        .wfd = STDOUT_FILENO,
        .rfd = STDIN_FILENO,
    };

    struct circuitS *c = NEW_C(errorcb);

    struct elementE *e = APPEND_A(c, promptA, "me@loop:~$ ");
                         APPEND_A(c, readA,   &io);
                         APPEND_A(c, echoA,   &io);
                         APPEND_A(c, writeA,  &io);
               loopA(e);

    /* Run circuitS */
    RUN_A(c, &counter, &pipe); 

    /* Start and wait for event loop */
    if (meloop_io_loop(NULL)) {
        err(1, "meloop_io_loop");
    }
    
    meloop_io_deinit();
    freeC(c);
    return EXIT_SUCCESS;
}
