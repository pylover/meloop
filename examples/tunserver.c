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
    INFO("[total clients: %u] %s, %s", *clients, 
            meloop_sockaddr_dump(&(conn->addr)), error);
   
    if (p != NULL) {
        if (p->buffer != NULL) {
            free(p->buffer);
        }
        free(p);
    }
}


void 
client_connected (struct circuitS *c, unsigned int *clients, int fd, 
        struct sockaddr *addr) {
    
    (*clients)++;
    INFO("[total clients: %u] %s", *clients, 
            meloop_sockaddr_dump(addr));

    /* Will be free at client_error() */
    struct pipeS *pipet = malloc(sizeof(struct pipeS));
    if (pipet == NULL) {
        close(fd);
        ERROR_A(c, clients, NULL, "Out of memory");
        return;
    }

    struct pipeS *pipes = malloc(sizeof(struct pipeS));
    if (pipes == NULL) {
        close(fd);
        ERROR_A(c, clients, NULL, "Out of memory");
        return;
    }

    pipet->wfd = fd; 
    pipes->rfd = fd; 

    pipet->buffer = malloc(CHUNK_SIZE);
    pipet->size = 0;

    pipes->buffer = malloc(CHUNK_SIZE);
    pipes->size = 0;
    
    // TODO: preserve ip address
    // memcpy(&(conn->addr), addr, sizeof(struct sockaddr));
    if (pipet->buffer == NULL) {
        ERROR_A(c, clients, NULL, "Out of memory");
        return;
    }

    if (pipes->buffer == NULL) {
        ERROR_A(c, clients, NULL, "Out of memory");
        return;
    }
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
                                   iot = NEW_C(NULL, client_error);
    struct elementE *et = APPEND_A(iot, pipereadA,  &iop);
                          APPEND_A(iot, pipewriteA, &iop);
               loopA(et);

    /* io socket reader circuiteS */
                                   ios = NEW_C(NULL, client_error);
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
    struct circuitS *circ = NEW_C(NULL, errorcb);

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
