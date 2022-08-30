#include "meloop/io.h"
#include "meloop/tcp.h"
#include "meloop/addr.h"
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
static struct circuitS *worker;


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
client_error(struct circuitS *c, unsigned int *clients, struct tcpconnS *conn, 
        const char *error) {
    (*clients)--;
    INFO("[total clients: %u] %s, %s", *clients, 
            meloop_sockaddr_dump(&(conn->remoteaddr)), error);
   
    if (conn->data != NULL) {
        if (conn->data->blob != NULL) {
            free(conn->data->blob);
        }
        free(conn->data);
    }
}


void 
client_connected (struct circuitS *c, unsigned int *clients, int fd, 
        struct sockaddr *addr) {
    
    (*clients)++;
    INFO("[total clients: %u] %s", *clients, 
            meloop_sockaddr_dump(addr));

    /* Will be free at client_error() */
    struct tcpconnS *conn = malloc(sizeof(struct tcpconnS));
    if (conn == NULL) {
        close(fd);
        ERROR_A(c, clients, NULL, "Out of memory");
        return;
    }

    conn->fd = fd; 
    memcpy(&(conn->remoteaddr), addr, sizeof(struct sockaddr));

    /* Will be free at tcp.c: client_free() */
    conn->data = malloc(sizeof(struct stringS));
    conn->data->blob = malloc(CHUNK_SIZE);
    conn->data->size = 0;
    
    if (conn->data->blob == NULL) {
        ERROR_A(c, clients, NULL, "Out of memory");
        return;
    }
    RUN_A(worker, clients, conn);
}


int main() {
    unsigned int clients = 0;
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);

    /* A circuitS to run for each new connection */
    struct ioP io = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
    };
    worker = NEW_C(client_error);
    struct elementE *e = APPEND_A(worker, readA,  &io);
                         APPEND_A(worker, writeA, &io);
               loopA(e);

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
    freeC(worker);
    freeC(circ);
    return status;
}
