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


int main() {
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);
    
    /* Initialize the buffer */
    static char b[CHUNK_SIZE];
    static struct tcpconnS conn = {
        .size = 0,
        .buffer = b,
        .fd = 4,
    };

    /* Initialize TCP Client */
    static struct tcpclientS tcp = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        .hostname = "127.0.0.1",
        .port = "9090"
    };
    
    static struct tunS tun = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        .tap = false,
        .fd = -1,
        .address = "192.168.11.2",
        .destaddress = "192.168.11.1",
        .netmask = "255.255.255.0",
    };

    /* Client init -> loop circuitS */
    struct circuitS *circ = NEW_C(NULL, errorcb);

                            APPEND_A(circ, tunopenA, &tun);
    //                      APPEND_A(circ, connectA, &tcp  );
    struct elementS *work = APPEND_A(circ, readA,    &tcp  );
                            APPEND_A(circ, writeA,   &tcp  );
               loopA(work);

    /* Run server circuitS */
    RUN_A(circ, NULL, &conn); 

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
