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
httpreqA(struct circuitS *c, struct ioS *io, struct stringS buff) {
    struct tlsclientS *priv = meloop_priv_ptr(c);
    char *b = buff.data;
    size_t s = 0;
    #define HCR "\r\n"
    s += sprintf(b + s, "GET / HTTP/1.1"HCR);
    s += sprintf(b + s, "Host: %s"HCR, priv->hostname);
    s += sprintf(b + s, "User-Agent: meloop/0.1 curl"HCR);
    s += sprintf(b + s, "Connection: close"HCR);
    s += sprintf(b + s, "Content-Length: 0"HCR);
    s += sprintf(b + s, "Accept: */*"HCR);
    s += sprintf(b + s, HCR);
    buff.size = s;
    RETURN_A(c, io, buff);
}


void
newlineA(struct circuitS *c, struct ioS *io, struct stringS buff) {
    buff.data[buff.size - 1] = '\n';
    RETURN_A(c, io, buff);
}


void
printA(struct circuitS *c, struct ioS *io, struct stringS buff) {
    printf("%.*s", (int)buff.size, buff.data);
    buff.size = 0;
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
    meloop_tls_init();

    // TODO: move to private params
    static struct tcpconnS conn = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
    };

    /* Initialize TCP Client */
    static struct tlsclientS tls = {
        //.hostname = "google.com",
        .hostname = "wttr.in",
        .port = "443",
    };
    
    /* Initialize the buffer */
    static char b[CHUNK_SIZE];
    struct stringS buff = {
        .size = 0,
        .data = b,
    };

    /* Client init -> loop circuitS */
    struct circuitS *circ = NEW_C(NULL, errorcb);

                            APPEND_A(circ, connectA,  meloop_ptr(&tls));
                            APPEND_A(circ, tlsA,      meloop_ptr(&tls));
    struct elementS *req  = APPEND_A(circ, httpreqA,  meloop_ptr(&tls));
                            APPEND_A(circ, tlswriteA, meloop_ptr(&tls));
    struct elementS *read = APPEND_A(circ, tlsreadA,  meloop_ptr(&tls));
                            APPEND_A(circ, printA,    NULL);
               loopA(read);

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
    
    meloop_tls_deinit();
    meloop_io_deinit();
    freeC(circ);
    return status;
}
