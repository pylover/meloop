#include "meloop/io.h"
#include "meloop/tcp.h"
#include "meloop/tls.h"
#include "meloop/random.h"
#include "meloop/addr.h"
#include "meloop/timer.h"
#include "meloop/logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>


#define CHUNK_SIZE  32
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
newlineA(struct circuitS *c, struct ioS *io, struct stringS buff) {
    buff.data[buff.size - 1] = '\n';
    RETURN_A(c, io, buff);
}


void
printA(struct circuitS *c, struct ioS *io, struct stringS buff) {
    printf("%.*s", (int)buff.size, buff.data);
    RETURN_A(c, io, buff);
}


void
errorcb(struct circuitS *c, struct ioS *s, union any data, 
        const char *error) {
    ERROR("%s", error);
    status = EXIT_FAILURE;
}


int main() {
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);

    // TODO: move to private params
    static struct tcpconnS conn = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
    };

    /* Initialize TCP Client */
    static struct tlsclientS tls = {
        .hostname = "google.com",
        .port = "443"
    };
    
    /* Initialize the buffer */
    static char b[CHUNK_SIZE];
    struct stringS buff = {
        .size = 0,
        .data = b,
    };

    /* Server init -> loop circuitS */
    struct circuitS *circ = NEW_C(NULL, errorcb);

                            APPEND_A(circ, connectA,   meloop_ptr(&tls));
                            APPEND_A(circ, tlsA,       meloop_ptr(&tls));
    struct elementS *work = APPEND_A(circ, writeA,      NULL);
                            APPEND_A(circ, readA,       NULL);
                            APPEND_A(circ, printA,      NULL);
               loopA(work);

    /* Run server circuitS */
    RUN_A(circ, &conn, buff); 

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
