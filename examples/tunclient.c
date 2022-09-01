#include "meloop/io.h"
#include "meloop/tcp.h"
#include "meloop/pipe.h"
#include "meloop/tuntap.h"
#include "meloop/tunpipe.h"
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
errorcb(struct circuitS *c, void *s, void *data, const char *error) {
    ERROR("%s", error);
    status = EXIT_FAILURE;
}


int main() {
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);
    
    static struct ioP io = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
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
    };

    static struct tcpconnS conn = {
        .fd = -1,
    };

    static struct tunS tun_ = {
        .fd = -1,
        .address = "192.168.11.2",
        .destaddress = "192.168.11.1",
        .netmask = "255.255.255.0",
    };

    /* Client init circuitS */
    struct circuitS *init = NEW_C(errorcb);
            APPEND_A(init, tt_connA, &tcp);
            APPEND_A(init, connectA, &tcp);
            APPEND_A(init, tt_ctxA,  NULL);
            tunpipeF(init, &io, &tun);

    /* Run client circuitS */
    run_tunpipeA(init, NULL, &conn, &tun_, CHUNK_SIZE);

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
    return status;
}
