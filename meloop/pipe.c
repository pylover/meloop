#include "meloop/pipe.h"
#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/logging.h"

#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>


void
pipereadA(struct circuitS *c, void *s, struct pipeS *pipe, 
        struct ioP *priv) {
    ssize_t size;

    /* Read from the file descriptor */
    size = read(pipe->rfd, pipe->data->blob, priv->readsize);

    /* Check for EOF */
    if (size == 0) {
        ERROR_A(c, s, pipe, "EOF");
        return;
    }

    /* Error | wait */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, pipe, pipe->rfd, EPOLLIN, priv->epollflags);
        }
        else {
            ERROR_A(c, s, pipe, "read");
        }
        return;
    }
    pipe->data->size = size;
    RETURN_A(c, s, pipe);
}


void
pipewriteA(struct circuitS *c, void *state, struct pipeS *pipe, 
        struct ioP *priv) {
    ssize_t size;
    
    size = write(pipe->wfd, pipe->data->blob, pipe->data->size);
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, state, pipe, pipe->wfd, EPOLLOUT, priv->epollflags);
        }
        else {
            ERROR_A(c, state, pipe, "write");
        }
        return;
    }
    RETURN_A(c, state, pipe);
}
