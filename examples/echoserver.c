#include "arrow/io.h"
#include "arrow/tcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/epoll.h>


#define CHUNK_SIZE  1024
static struct circuit *worker;


void
errorcb(struct circuit *c, struct tcpserver *s, const char *error) {
    perror(error);
}


void
successcb(struct circuit *c, struct tcpserver *s, int out) {
    printf("Out: %d\n", out);
}


void
clinet_error(struct circuit *c, struct conn *conn, struct string buff, 
        const char *error) {
    printf("clinet error: %s\n", error);
   
    if (buff.size) {
        free(buff.data);
    }
    if (conn != NULL) {
        free(conn);
    }
}


void 
client_connected (struct circuit *c, struct tcpserver *s, int fd, 
        struct sockaddr *addr) {
    printf("new client\n");

    /* Will be free at tcp: client_free() */
    struct conn *conn = malloc(sizeof(struct conn));
    if (conn == NULL) {
        close(fd);
        ERROR_A(c, s, NULL, "Out of memory");
        return;
    }

    // printf("new conn\n");
    conn->rfd = fd; 
    conn->wfd = fd; 
    conn->readsize = s->readsize; 
    memcpy(&(conn->addr), addr, sizeof(struct sockaddr_in));

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
    arrow_io_init(0);

    worker = NEW_C(NULL, clinet_error);
    struct element *e = APPEND_A(worker, readA,  NULL);
                        APPEND_A(worker, writeA, NULL);
              loopA(e);


    static struct tcpserver server = {
        .epollflags = EPOLLET, 
        .readsize = 1024,
        .host = "127.0.0.1",
        .port = 9090,
        .backlog = 2,
        .client_connected = client_connected,
        // .worker = echoloopF(&client_props),
        // .client_closed = client_closed
    };
    
    struct circuit *circ = NEW_C(successcb, errorcb);

                           APPEND_A(circ, listenA, NULL);
    struct element *acpt = APPEND_A(circ, acceptA, NULL);
              loopA(acpt);

    /* Run circuit */
    runA(circ, &server, any_null()); 

    /* Start and wait for event loop */
    if (arrow_io_loop(NULL)) {
        err(1, "arrow_io_loop");
    }
    printf("after loop\n");

    // if (monad_tcp_runserver(&bindinfo, finish, &status)) {
    //     status = EXIT_FAILURE;
    // }
    //printf("after loop\n");
    //monad_free(bindinfo.worker);
    //monad_io_deinit();
    //return status;
    arrow_io_deinit();
    freeC(circ);
    return EXIT_SUCCESS;
}
