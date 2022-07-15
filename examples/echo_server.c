#include "monad/monad.h"
#include "monad/io.h"
#include "monad/tcp.h"

#include <stdlib.h>
#include <sys/epoll.h>


#define CHUNK_SIZE  1024
#define ERROR -1
#define OK 0
static volatile int status = OK;


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
    struct sockaddr_in addr = c->addr;
    printf("%s:%d Connected.\n", inet_ntoa(addr.sin_addr), 
            addr.sin_port);
}


static void client_closed(MonadContext *ctx, struct conn *c, 
        const char *reason) {
    struct sockaddr_in addr = c->addr;
    
    if (reason != NULL) {
        printf("%s:%d Error: %s\n", inet_ntoa(addr.sin_addr), 
            addr.sin_port, reason);
    }
    else {
        printf("%s:%d Disconnected.\n", inet_ntoa(addr.sin_addr), 
            addr.sin_port);
    }
}


int main() {
    monad_io_init(0);
    
    static struct io_props client_props = {
        .epollflags = EPOLLET, 
        .readsize = CHUNK_SIZE
    };
    
    struct bind bindinfo = {
        .host = "127.0.0.1",
        .port = 9090,
        .backlog = 2,
        .client_monad = echoF(&client_props),
        .client_connected = client_connected,
        .client_closed = client_closed
    };
    
    monad_tcp_runserver(&bindinfo, finish);
    monad_free(bindinfo.client_monad);
    monad_io_deinit();
    return status;
}
