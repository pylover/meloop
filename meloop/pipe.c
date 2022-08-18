#include "meloop/pipe.h"
#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/logging.h"

#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>


void
pipereadA(struct circuitS *c, void *state, struct pipeS *pipe) {
    readA(c, state, (struct fileS*)pipe);
}


void
pipewriteA(struct circuitS *c, void *state, struct pipeS *pipe) {
    ssize_t size;
    
    size = write(pipe->wfd, pipe->buffer, pipe->size);
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, state, pipe, pipe->wfd, EPOLLOUT);
        }
        else {
            ERROR_A(c, state, pipe, "write");
        }
        return;
    }
    RETURN_A(c, state, pipe);
}
