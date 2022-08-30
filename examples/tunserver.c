#include "meloop/io.h"
#include "meloop/tcp.h"
#include "meloop/addr.h"
#include "meloop/pipe.h"
#include "meloop/logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/epoll.h>


#define CHUNK_SIZE  1024
#define WORKING 9999
static volatile int status = WORKING;
static struct sigaction old_action;
static struct circuitS *iot = NULL;
static struct circuitS *ios = NULL;


void sighandler(int s) {
    status = EXIT_SUCCESS;
}


void catch_signal() {
    struct sigaction new_action = {sighandler, 0, 0, 0, 0};
    if (sigaction(SIGINT, &new_action, &old_action) != 0) {
        err(EXIT_FAILURE, NULL);
    }
}


void
errorcb(struct circuitS *c, struct tcpserverP *s, void *data, 
        const char *error) {
    ERROR("%s", error);
    status = EXIT_FAILURE;
}


void
client_error(struct circuitS *c, unsigned int *clients, struct pipeS *p, 
        const char *error) {
    (*clients)--;
    // preserve address
    // INFO("[total clients: %u] %s, %s", *clients, 
    //         meloop_sockaddr_dump(&(conn->addr)), error);
   
    if (p->data != NULL) {
        if (p->data->blob != NULL) {
            free(p->data->blob);
        }
        free(p->data);
    }
}


void 
client_connected (struct circuitS *c, unsigned int *clients, int fd, 
        struct sockaddr *addr) {
    
    (*clients)++;
    INFO("[total clients: %u] %s", *clients, 
            meloop_sockaddr_dump(addr));

    /* Will be free at client_error() */
    struct stringS *datat = malloc(sizeof(struct stringS));
    datat->blob = malloc(CHUNK_SIZE);
    datat->size = 0;
    struct pipeS *pipet = malloc(sizeof(struct pipeS));
    pipet->data = datat; 
    pipet->wfd = fd; 

    struct stringS *datas = malloc(sizeof(struct stringS));
    datas->blob = malloc(CHUNK_SIZE);
    datas->size = 0;
    struct pipeS *pipes = malloc(sizeof(struct pipeS));
    pipes->data = datas; 
    pipes->rfd = fd; 
    
    // TODO: preserve ip address
    // memcpy(&(conn->addr), addr, sizeof(struct sockaddr));
    RUN_A(iot, clients, &pipet); 
    RUN_A(ios, clients, &pipes); 
}


int main() {
    unsigned int clients = 0;
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);

    /* io tun reader circuiteS */
    struct ioP iop = {EPOLLET, CHUNK_SIZE};
                                   iot = NEW_C(client_error);
    struct elementE *et = APPEND_A(iot, pipereadA,  &iop);
                          APPEND_A(iot, pipewriteA, &iop);
               loopA(et);

    /* io socket reader circuiteS */
                                   ios = NEW_C(client_error);
    struct elementE *es = APPEND_A(ios, pipereadA,  &iop);
                          APPEND_A(ios, pipewriteA, &iop);
               loopA(es);

    /* Initialize TCP Server */
    static struct tcpserverP server = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
        .backlog = 2,
        .bindaddr = "127.0.0.1",
        .bindport = 9090,
        .client_connected = (meloop_tcpserver_conn_event)client_connected,
    };
    
    static struct fileS file = {
        .fd = -1
    };

    /* Server init -> loop circuitS */
    struct circuitS *circ = NEW_C(errorcb);

                            APPEND_A(circ, listenA, &server);
    struct elementE *acpt = APPEND_A(circ, acceptA, &server);
               loopA(acpt);

    /* Run server circuitS */
    RUN_A(circ, &clients, &file); 

    /* Start and wait for event loop */
    if (meloop_io_loop(&status)) {
        ERROR("meloop_io_loop");
        status = EXIT_FAILURE;
    }
    else {
        status = EXIT_SUCCESS;
    }

    meloop_io_deinit();
    freeC(iot);
    freeC(ios);
    freeC(circ);
    return status;
}
