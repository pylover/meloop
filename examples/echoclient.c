#include "meloop/io.h"
#include "meloop/tcp.h"
#include "meloop/random.h"
#include "meloop/addr.h"
#include "meloop/timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>


#define CHUNK_SIZE  32


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


void connected(struct circuitS *c, struct ioS *s, struct sockaddr *a) {
    printf("Connected\n");
}


void
errorcb(struct circuitS *c, struct ioS *s, union any data, 
        const char *error) {
    printf("%s\n", error);
    perror(error);
}


void
successcb(struct circuitS *c, struct ioS *s, int out) {
    printf("Out: %d\n", out);
}


int main() {
    int status;
    meloop_io_init(0);

    // TODO: move to private params
    static struct tcpconnS conn = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
    };


    /* Initialize TCP Client */
    static struct tcpclientS tcp = {
        .connected = connected,
        .hostname = "127.0.0.1",
        .port = "9090"
    };
    
    /* Initialize random settings. */
    static struct randS rand = {
        .fd = -1
    };

    /* Initialize timer settings. */
    #define S  1000000000 
    #define MS 1000000 
    static struct timerS timer = {
        .clockid = CLOCK_REALTIME,
        .flags = 0,
        .fd = -1,
        .interval_ns = 1 * MS,
    };

    /* Initialize the buffer */
    static char b[CHUNK_SIZE];
    struct stringS buff = {
        .size = 0,
        .data = b,
    };

    /* Server init -> loop circuitS */
    struct circuitS *circ = NEW_C(successcb, errorcb);

                            APPEND_A(circ, timeropenA,  meloop_ptr(&timer));
                            APPEND_A(circ, connectA,    meloop_ptr(&tcp));
                            APPEND_A(circ, randopenA,   meloop_ptr(&rand));
    struct elementS *work = APPEND_A(circ, randreadA,   meloop_ptr(&rand));
                            APPEND_A(circ, randencA,    meloop_ptr(&rand));
                            APPEND_A(circ, newlineA,    NULL);
                            APPEND_A(circ, writeA,      NULL);
                            APPEND_A(circ, readA,       NULL);
                            APPEND_A(circ, printA,      NULL);
                            APPEND_A(circ, timersleepA, meloop_ptr(&timer));
               loopA(work);

    /* Run server circuitS */
    RUN_A(circ, &conn, buff); 

    /* Start and wait for event loop */
    if (meloop_io_loop(NULL)) {
        perror("meloop_io_loop");
        status = EXIT_FAILURE;
    }
    else {
        status = EXIT_SUCCESS;
    }

    meloop_io_deinit();
    freeC(circ);
    return status;
}
