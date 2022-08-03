#include "arrow/io.h"
#include "arrow/tcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <err.h>
#include <sys/epoll.h>


#define CHUNK_SIZE  1024


void
errorcb(struct circuit *c, struct tcpserver *s, const char *error) {
    dprintf(STDERR_FILENO, "%s\n", error);
}


void
successcb(struct circuit *c, struct tcpserver *s, int out) {
    printf("Out: %d\n", out);
}


int main() {
    arrow_io_init(0);
        
    static struct tcpserver server = {
        .host = "127.0.0.1",
        .port = 9090,
        .backlog = 2,

        .epollflags = EPOLLET, 

        // .worker = echoloopF(&client_props),
        // .client_connected = client_connected,
        // .client_closed = client_closed
    };
    
    
    struct circuit *circ = NEW_C(successcb, errorcb);

                           APPEND_A(circ, listenA, NULL);
    struct element *acpt = APPEND_A(circ, acceptA, NULL);
              loopA(acpt);

    // if (monad_tcp_runserver(&bindinfo, finish, &status)) {
    //     status = EXIT_FAILURE;
    // }
    //printf("after loop\n");
    //monad_free(bindinfo.worker);
    //monad_io_deinit();
    //return status;
    return EXIT_SUCCESS;
}
