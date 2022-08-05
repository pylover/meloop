#include "arrow/io.h"
#include "arrow/tcp.h"
#include "arrow/addr.h"

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
static struct circuit *worker;


void sighandler(int s) {
    printf("\nSIGINT detected: %d\n", s);
    status = EXIT_SUCCESS;
}


void catch_signal() {
    struct sigaction new_action = {sighandler, 0, 0, 0, 0};
    if (sigaction(SIGINT, &new_action, &old_action) != 0) {
        err(EXIT_FAILURE, NULL);
    }
}


void
errorcb(struct circuit *c, struct tcpserver *s, union any data, 
        const char *error) {
    perror(error);
    status = EXIT_FAILURE;
}


void
successcb(struct circuit *c, struct tcpserver *s, int out) {
    printf("Out: %d\n", out);
}


void
client_error(struct circuit *c, struct conn *conn, struct string buff, 
        const char *error) {
    printf("Clinet disconnected: %s -- %s\n", addr_dump(&(conn->addr)), error);
   
    if (buff.data != NULL) {
        free(buff.data);
    }
    if (conn != NULL) {
        free(conn);
    }
}


void 
client_connected (struct circuit *c, struct tcpserver *s, int fd, 
        struct sockaddr *addr) {
    
    printf("Client connected: %s\n", addr_dump(addr));

    /* Will be free at tcp: client_free() */
    struct conn *conn = malloc(sizeof(struct conn));
    if (conn == NULL) {
        close(fd);
        ERROR_A(c, s, NULL, "Out of memory");
        return;
    }

    conn->rfd = fd; 
    conn->wfd = fd; 
    conn->epollflags = EPOLLET;
    conn->readsize = s->readsize; 
    memcpy(&(conn->addr), addr, sizeof(addr));

    /* Will be free at tcp.c: client_free() */
    struct string buff = {
        .size = 0,
        .data = malloc(s->readsize),
    };

    if (buff.data == NULL) {
        ERROR_A(c, s, NULL, "Out of memory");
        return;
    }
    conn->server = s;
    runA(worker, conn, any_string(buff));
}


int main() {
    catch_signal();
    arrow_io_init(0);

    /* A circuit to run for each new connection */
    worker = NEW_C(NULL, client_error);
    struct element *e = APPEND_A(worker, readA,  NULL);
                        APPEND_A(worker, writeA, NULL);
              loopA(e);

    /* Initialize TCP Server */
    static struct tcpserver server = {
        .epollflags = EPOLLET,
        .readsize = 1024,
        .backlog = 2,
        .client_connected = client_connected,
    };

    /* Parse listen address */
    addr_parse(&(server.bind), "127.0.0.1", 9090);
    
    /* Server init -> loop circuit */
    struct circuit *circ = NEW_C(successcb, errorcb);

                           APPEND_A(circ, listenA, NULL);
    struct element *acpt = APPEND_A(circ, acceptA, NULL);
              loopA(acpt);

    /* Run server circuit */
    runA(circ, &server, any_null()); 

    /* Start and wait for event loop */
    if (arrow_io_loop(&status)) {
        perror("arrow_io_loop");
        status = EXIT_SUCCESS;
    }

    arrow_io_deinit();
    freeC(worker);
    freeC(circ);
    return status;
}
