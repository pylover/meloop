#include "meloop/io.h"
#include "meloop/tcp.h"
#include "meloop/tuntap.h"
#include "meloop/logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>


#define CHUNK_SIZE  1440 
#define WORKING 9999
static volatile int status = WORKING;
static struct sigaction old_action;
static struct circuitS *worker;


void sighandler(int s) {
    PRINTE(CR);
    status = EXIT_SUCCESS;
}


void catch_signal() {
    struct sigaction new_action = {sighandler, 0, 0, 0, 0};
    if (sigaction(SIGINT, &new_action, &old_action) != 0) {
        err(EXIT_FAILURE, NULL);
    }
}


void
errorcb(struct circuitS *c, void *s, void *data, const char *error) {
    ERROR("%s", error);
    status = EXIT_FAILURE;
}


struct pipesS {
        size_t lsize;
        char *lbuffer;
        int lfd;

        size_t rsize;
        char *rbuffer;
        int rfd;
};


void
rightA(struct circuitS *c, void *s, struct pipesS *pipes) {
    struct tunS *priv = meloop_priv_ptr(c);
    pipes->rfd = priv->fd;
    RETURN_A(c, s, pipes);
}


void
readpipesA(struct circuitS *c, void *s, struct fileS *f) {
    struct ioS *priv = meloop_priv_ptr(c);
    ssize_t size;

    /* Read from the file descriptor */
    size = read(f->fd, f->buffer, priv->readsize);

    /* Check for EOF */
    if (size == 0) {
        ERROR_A(c, s, f, "EOF");
        return;
    }

    /* Error | wait */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, f, f->fd, EPOLLIN);
        }
        else {
            ERROR_A(c, s, f, "read");
        }
        return;
    }
    f->size = size;
    RETURN_A(c, s, f);
}


int main() {
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);
    
    /* Initialize the buffer */
    static char lb[CHUNK_SIZE];
    static char rb[CHUNK_SIZE];
    static struct pipesS pipes = {
        .lsize = 0,
        .lbuffer = lb,
        .lfd = -1,

        .rsize = 0,
        .rbuffer = rb,
        .rfd = -1,
    };

    /* Initialize TCP Client */
    static struct tcpclientP tcp = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        .hostname = "127.0.0.1",
        .port = "9090"
    };
    
    static struct tunP tun = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        .tap = false,
        .fd = -1,
        .address = "192.168.11.2",
        .destaddress = "192.168.11.1",
        .netmask = "255.255.255.0",
    };
    
    // TODO: make tun nonblock
    /* Client init -> loop circuitS */
    struct circuitS *circ = NEW_C(NULL, errorcb);

                            APPEND_A(circ, tunopenA,    &tun);
                            APPEND_A(circ, rightA,      &tcp);
                            APPEND_A(circ, connectA,    &tcp);
    // struct elementS *work = APPEND_A(circ, readpipesA,  &tcp);
    //                         APPEND_A(circ, writepipesA, &tcp);
    //            loopA(work);

    /* Run server circuitS */
    RUN_A(circ, NULL, &pipes); 

    /* Start and wait for event loop */
    if (meloop_io_loop(&status)) {
        ERROR("meloop_io_loop");
        status = EXIT_FAILURE;
    }
    else {
        status = EXIT_SUCCESS;
    }
    
    meloop_io_deinit();
    freeC(circ);
    return status;
}
