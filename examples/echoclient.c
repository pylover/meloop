#include "meloop/io.h"
#include "meloop/tcp.h"
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
newlineA(struct circuitS *c, struct fileS *io, struct stringS buff) {
    buff.data[buff.size - 1] = '\n';
    RETURN_A(c, io, buff);
}


void
printA(struct circuitS *c, struct fileS *io, struct stringS buff) {
    printf("%.*s", (int)buff.size, buff.data);
    RETURN_A(c, io, buff);
}


void
errorcb(struct circuitS *c, struct fileS *s, union any data, 
        const char *error) {
    ERROR("%s", error);
    status = EXIT_FAILURE;
}


// struct state {
//     unsigned int lines;
// };


int main() {
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);
    
    static struct tcpconnS conn = {
    };

    /* Initialize TCP Client */
    static struct tcpclientS tcp = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        .hostname = "127.0.0.1",
        .port = "9090"
    };
    
    /* Initialize random settings. */
    static struct randS rand = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        .fd = -1
    };

    /* Initialize timer settings. */
    #define S  ((long)1000000000)
    #define MS ((long)1000000)
    static struct timerS timer = {
        .clockid = CLOCK_REALTIME,
        .flags = 0,
        .fd = -1,
        .interval_ns = S,
    };

    /* Initialize the buffer */
    static char b[CHUNK_SIZE];
    struct stringS buff = {
        .size = 0,
        .data = b,
    };

    /* Client init -> loop circuitS */
    struct circuitS *circ = NEW_C(NULL, errorcb);

                            APPEND_A(circ, timeropenA,  meloop_ptr(&timer));
                            APPEND_A(circ, connectA,    meloop_ptr(&tcp));
                            APPEND_A(circ, randopenA,   meloop_ptr(&rand));
    struct elementS *work = APPEND_A(circ, randreadA,   meloop_ptr(&rand));
                            APPEND_A(circ, randencA,    meloop_ptr(&rand));
                            APPEND_A(circ, newlineA,    NULL);
                            APPEND_A(circ, writeA,      meloop_ptr(&tcp));
                            APPEND_A(circ, readA,       meloop_ptr(&tcp));
                            APPEND_A(circ, printA,      NULL);
                            APPEND_A(circ, timersleepA, meloop_ptr(&timer));
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
