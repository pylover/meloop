#include "meloop/io.h"
#include "meloop/tcp.h"
#include "meloop/pipe.h"
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
static struct circuitS *iot = NULL;
static struct circuitS *ios = NULL;
static char bt[CHUNK_SIZE] = "\0";
static char bs[CHUNK_SIZE] = "\0";
static struct pipeS pipet = {bt, 0, -1, -1};
static struct pipeS pipes = {bs, 0, -1, -1};


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
inittunA(struct circuitS *c, void *s, void *d, struct tunP *priv) {
    pipet.rfd = priv->fd;
    pipes.wfd = priv->fd;
    RETURN_A(c, s, d);
}


void
errorcb(struct circuitS *c, void *s, void *data, const char *error) {
    ERROR("%s", error);
    status = EXIT_FAILURE;
}


void
initcb(struct circuitS *c, void *s, void *d) {
    RUN_A(iot, NULL, &pipet); 
    RUN_A(ios, NULL, &pipes); 
}


int main() {
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);
    
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
    /* Client init circuitS */
    struct circuitS *init = NEW_C(initcb, errorcb);
            APPEND_A(init, connectA, &tcp);
            APPEND_A(init, tunopenA, &tun);
            APPEND_A(init, inittunA, &tcp);

    /* io tun reader circuiteS */
    struct ioP iop = {EPOLLET, CHUNK_SIZE};
                                   iot = NEW_C(NULL, NULL);
    struct elementE *et = APPEND_A(iot, pipereadA,  &iop);
                          APPEND_A(iot, pipewriteA, &iop);
               loopA(et);

    /* io socket reader circuiteS */
                                   ios = NEW_C(NULL, NULL);
    struct elementE *es = APPEND_A(ios, pipereadA,  &iop);
                          APPEND_A(ios, pipewriteA, &iop);
               loopA(es);

    /* Run client circuitS */
    RUN_A(init, NULL, &pipes); 

    /* Start and wait for event loop */
    if (meloop_io_loop(&status)) {
        ERROR("meloop_io_loop");
        status = EXIT_FAILURE;
    }
    else {
        status = EXIT_SUCCESS;
    }
    
    meloop_io_deinit();
    freeC(init);
    freeC(iot);
    freeC(ios);
    return status;
}
