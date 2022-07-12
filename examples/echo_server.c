#include "monad/monad.h"
#include "monad/io.h"
#include "monad/tcp.h"

#include <stdlib.h>


#define CHUNK_SIZE  1024
#define ERROR -1
#define OK 0
static volatile int status = OK;
static struct device clientdev = {false, CHUNK_SIZE};


static void finish(MonadContext *ctx, struct conn *c, 
        const char *reason) {
    if (reason != NULL) {
        perror(reason);
        status = ERROR;
    }
    status = OK;
}


static void client_connected(MonadContext *ctx, struct conn *c, 
        const char *reason) {
    printf("Client connected: %d\n", c->rfd);
}


static void client_closed(MonadContext *ctx, struct conn *c, 
        const char *reason) {
    if (reason != NULL) {
        printf("client error: %d %s\n", c->rfd, reason);
    }
    else {
        printf("client closed: %d\n", c->rfd);
    }
}


int main() {
    mio_init(0);
    
    struct bind bindinfo = {
        .host = "127.0.0.1",
        .port = 9090,
        .backlog = 2,
        .client_monad = echoF(&clientdev),
        .client_connected = client_connected,
        .client_closed = client_closed
    };
    
    monad_tcp_runserver(&bindinfo, finish);
    monad_free(bindinfo.client_monad);
    mio_deinit();
    return status;
}
