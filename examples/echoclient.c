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


struct ctx {
    struct timerS *timer;
    struct tcpconnS *conn;
    struct fileS *rand;
};


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
randA(struct circuitS *c, unsigned int *state, struct ctx *ctx) {
    RETURN_A(c, state, ctx->rand);
}


void
timerA(struct circuitS *c, unsigned int *state, struct ctx *ctx) {
    RETURN_A(c, state, ctx->timer);
}


void
connA(struct circuitS *c, unsigned int *state, struct ctx *ctx) {
    RETURN_A(c, state, ctx->conn);
}


void
ctxA(struct circuitS *c, unsigned int *state, struct fileS *file) {
    RETURN_A(c, state, file->ptr);
}


void
newlineA(struct circuitS *c, int *state, struct fileS *f) {
    (*state)++;
    f->data->blob[f->data->size - 1] = '\n';
    RETURN_A(c, state, f);
}


void
greetingA(struct circuitS *c, int *state, struct tcpconnS *conn) {
    struct sockaddr *la = &(conn->localaddr);
    struct sockaddr *ra = &(conn->remoteaddr);
    printf("Successfully connected from %s", meloop_sockaddr_dump(la));
    printf(" to %s.\n", meloop_sockaddr_dump(ra));
    RETURN_A(c, state, conn);
}


void
printA(struct circuitS *c, int *state, struct fileS *f) {
    printf("%03d %.*s", *state, (int)f->data->size, f->data->blob);
    RETURN_A(c, state, f);
}


void
errorcb(struct circuitS *c, struct fileS *s, void *data, const char *error) {
    ERROR("%s", error);
    status = EXIT_FAILURE;
}


int main() {
    logging_verbosity = LOGGING_DEBUG;
    static struct ctx ctx;
    catch_signal();
    meloop_io_init(0);
    int state = 0;
    
    /* Initialize the buffer */
    static char b[CHUNK_SIZE];
    static struct stringS data = {
        .blob = b,
        .size = 0,
    };

    static struct tcpconnS conn = {
        .data = &data,
        .fd = -1,
        .ptr = &ctx,
    };
    ctx.conn = &conn;

    static struct timerS tmr = {
        .fd = -1,
        .ptr = &ctx,
    };
    ctx.timer = &tmr;

    static struct fileS rnd = {
        .fd = -1,
        .data = &data,
        .ptr = &ctx,
    };
    ctx.rand = &rnd;

    /* Initialize TCP Client */
    static struct tcpclientP tcp = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        .hostname = "127.0.0.1",
        .port = "9090"
    };
    
    /* Initialize random settings. */
    static struct ioP rand = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
    };

    /* Initialize timer settings. */
    #define S  ((long)1000000000)
    #define MS ((long)1000000)
    static struct timerP timer = {
        .epollflags = EPOLLET,
        .clockid = CLOCK_REALTIME,
        .flags = 0,
        .interval_ns = S,
    };


    /* Client init -> loop circuitS */
    struct circuitS *circ = NEW_C(errorcb);

                            APPEND_A(circ, timerA,      &timer);
                            APPEND_A(circ, timeropenA,  &timer);

                            APPEND_A(circ, ctxA,        NULL  );
                            APPEND_A(circ, connA,       NULL  );
                            APPEND_A(circ, connectA,    &tcp  );
                            APPEND_A(circ, greetingA,   &tcp  );

                            APPEND_A(circ, ctxA,        NULL  );
                            APPEND_A(circ, randA,       NULL  );
                            APPEND_A(circ, randopenA,   &rand );
    struct elementE *work = APPEND_A(circ, randreadA,   &rand );
                            APPEND_A(circ, randencA,    &rand );
                            APPEND_A(circ, newlineA,    NULL  );

                            APPEND_A(circ, ctxA,        NULL  );
                            APPEND_A(circ, connA,       NULL  );
                            APPEND_A(circ, writeA,      &tcp  );
                            APPEND_A(circ, readA,       &tcp  );
                            APPEND_A(circ, printA,      NULL  );

                            APPEND_A(circ, ctxA,        NULL  );
                            APPEND_A(circ, timerA,      &timer);
                            APPEND_A(circ, timersleepA, &timer);

                            APPEND_A(circ, ctxA,        NULL  );
                            APPEND_A(circ, randA,       NULL  );
               loopA(work);

    /* Run server circuitS */
    RUN_A(circ, &state, &ctx); 

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
