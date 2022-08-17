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


struct state {
    size_t clients;
};


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
errorcb(struct circuitS *c, struct tcpserverS *s, void *data, 
        const char *error) {
    ERROR("%s", error);
    status = EXIT_FAILURE;
}


void
client_error(struct circuitS *c, struct state *state, struct tcpconnS *conn, 
        const char *error) {
    INFO("%s, %s", meloop_addr_dump(&(conn->addr)), error);
   
    if (conn->buffer != NULL) {
        free(conn->buffer);
    }
}


void 
client_connected (struct circuitS *c, struct state *state, int fd, 
        struct sockaddr *addr) {
    
    INFO("%s", meloop_addr_dump(addr));

    /* Will be free at client_error() */
    struct tcpconnS *conn = malloc(sizeof(struct tcpconnS));
    if (conn == NULL) {
        close(fd);
        ERROR_A(c, state, NULL, "Out of memory");
        return;
    }

    conn->fd = fd; 
    memcpy(&(conn->addr), addr, sizeof(struct sockaddr));

    /* Will be free at tcp.c: client_free() */
    conn->buffer = malloc(CHUNK_SIZE);
    conn->size = 0;
    
    if (conn->buffer == NULL) {
        ERROR_A(c, state, NULL, "Out of memory");
        return;
    }
    RUN_A(worker, state, conn);
}


int main() {
    logging_verbosity = LOGGING_DEBUG;
    catch_signal();
    meloop_io_init(0);

    /* A circuitS to run for each new connection */
    struct ioS io = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE,
    };
    worker = NEW_C(NULL, client_error);
    struct elementS *e = APPEND_A(worker, readA,  &io);
                         APPEND_A(worker, writeA, &io);
               loopA(e);

    /* Initialize TCP Server */
    static struct tcpserverS server = {
        .backlog = 2,
        .client_connected = (meloop_tcpserver_conn_event)client_connected,
    };
    
    static struct fileS file = {
        .fd = -1
    };

    static struct state state = {
        .clients = 0 
    };

    /* Parse listen address */
    meloop_addr_parse(&(server.bind), "127.0.0.1", 9090);
    
    /* Server init -> loop circuitS */
    struct circuitS *circ = NEW_C(NULL, errorcb);

                            APPEND_A(circ, listenA, &server);
    struct elementS *acpt = APPEND_A(circ, acceptA, &server);
               loopA(acpt);

    /* Run server circuitS */
    RUN_A(circ, &state, &file); 

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
