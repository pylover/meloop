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
httpreqA(struct circuitS *c, void *s, struct stringS *data) {
    struct tlsclientS *priv = meloop_priv_ptr(c);
    char *b = data->buffer;
    size_t l = 0;
    #define HCR "\r\n"
    l += sprintf(b + l, "GET / HTTP/1.1"HCR);
    l += sprintf(b + l, "Host: %s"HCR, priv->hostname);
    l += sprintf(b + l, "User-Agent: meloop/0.1 curl"HCR);
    l += sprintf(b + l, "Connection: close"HCR);
    l += sprintf(b + l, "Content-Length: 0"HCR);
    l += sprintf(b + l, "Accept: */*"HCR);
    l += sprintf(b + l, HCR);
    data->size = l;
    RETURN_A(c, s, data);
}


void
newlineA(struct circuitS *c, void *s, struct stringS *data) {
    data->buffer[data->size - 1] = '\n';
    RETURN_A(c, s, data);
}


void
printA(struct circuitS *c, void *s, struct stringS *data) {
    printf("%.*s", (int)data->size, data->buffer);
    data->size = 0;
    RETURN_A(c, s, data);
}


void
errorcb(struct circuitS *c, struct fileS *s, void *data, 
        const char *error) {
    ERROR("%s", error);
    status = EXIT_FAILURE;
}


int main() {
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);
    meloop_tls_init();

    /* Initialize the buffer */
    static char b[CHUNK_SIZE];
    static struct tcpconnS conn = {
        .size = 0,
        .buffer = b,
    };

    /* Initialize TCP Client */
    static struct tlsclientS tls = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        // .hostname = "google.com",
        .hostname = "wttr.in",
        .port = "443",
    };
    
    /* Client init -> loop circuitS */
    struct circuitS *circ = NEW_C(NULL, errorcb);

                            APPEND_A(circ, connectA,  &tls);
                            APPEND_A(circ, tlsA,      &tls);
    struct elementE *req  = APPEND_A(circ, httpreqA,  &tls);
                            APPEND_A(circ, tlswriteA, &tls);
    struct elementE *read = APPEND_A(circ, tlsreadA,  &tls);
                            APPEND_A(circ, printA,    NULL);
               loopA(read);

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
    
    meloop_tls_deinit();
    meloop_io_deinit();
    freeC(circ);
    return status;
}
