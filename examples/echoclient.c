#include "meloop/io.h"
#include "meloop/tcp.h"
#include "meloop/random.h"
#include "meloop/addr.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/epoll.h>


#define CHUNK_SIZE  1024


void connected(struct circuitS *c, struct tcpclientS *s, struct sockaddr *a) {
    printf("Connected\n");
}


void
errorcb(struct circuitS *c, struct tcpserverS *s, union any data, 
        const char *error) {
    perror(error);
}


void
successcb(struct circuitS *c, struct tcpserverS *s, int out) {
    printf("Out: %d\n", out);
}


int main() {
    int status;
    meloop_io_init(0);

    /* Initialize TCP Server */
    static struct tcpclientS client = {
        .epollflags = EPOLLET,
        .readsize = 1024,
        .connected = connected,
        .hostname = "127.0.0.1",
        .port = "9090"
    };

    /* Initialize the buffer */
    static char b[CHUNK_SIZE];
    struct stringS buff = {
        .size = CHUNK_SIZE,
        .data = b,
    };

    /* Server init -> loop circuitS */
    struct circuitS *circ = NEW_C(successcb, errorcb);

    //                         APPEND_A(circ, connectA,  NULL);
                            APPEND_A(circ, randopenA, NULL);
    struct elementS *work = APPEND_A(circ, randreadA, NULL);
                            APPEND_A(circ, writeA,    NULL);
                            APPEND_A(circ, readA,     NULL);
    //                         APPEND_A(circ, printA,    NULL);
    //                         APPEND_A(circ, sleepA,    1000);
               loopA(work);

    /* Run server circuitS */
    RUN_A(circ, &client, buff); 

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
