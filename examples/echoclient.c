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
newlineA(struct circuitS *c, int *state, struct stringS *data) {
    (*state)++;
    data->buffer[data->size - 1] = '\n';
    RETURN_A(c, state, data);
}


void
printA(struct circuitS *c, int *state, struct stringS *data) {
    printf("%03d %.*s", *state, (int)data->size, data->buffer);
    RETURN_A(c, state, data);
}


void
errorcb(struct circuitS *c, struct fileS *s, void *data, const char *error) {
    ERROR("%s", error);
    status = EXIT_FAILURE;
}


int main() {
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);
    int state = 0;
    
    /* Initialize the buffer */
    static char b[CHUNK_SIZE];
    static struct tcpconnS conn = {
        .size = 0,
        .buffer = b,
        .fd = -1,
    };

    /* Initialize TCP Client */
    static struct tcpclientP tcp = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        .hostname = "127.0.0.1",
        .port = "9090"
    };
    
    /* Initialize random settings. */
    static struct randP rand = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        .fd = -1
    };

    /* Initialize timer settings. */
    #define S  ((long)1000000000)
    #define MS ((long)1000000)
    static struct timerP timer = {
        .epollflags = EPOLLET,
        .clockid = CLOCK_REALTIME,
        .flags = 0,
        .fd = -1,
        .interval_ns = S,
    };

    /* Client init -> loop circuitS */
    struct circuitS *circ = NEW_C(NULL, errorcb);

                            APPEND_A(circ, timeropenA,  &timer);
                            APPEND_A(circ, connectA,    &tcp  );
                            APPEND_A(circ, randopenA,   &rand );
    struct elementE *work = APPEND_A(circ, randreadA,   &rand );
                            APPEND_A(circ, randencA,    &rand );
                            APPEND_A(circ, newlineA,    NULL  );
                            APPEND_A(circ, writeA,      &tcp  );
                            APPEND_A(circ, readA,       &tcp  );
                            APPEND_A(circ, printA,      NULL  );
                            APPEND_A(circ, timersleepA, &timer);
               loopA(work);

    /* Run server circuitS */
    RUN_A(circ, &state, &conn); 

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
