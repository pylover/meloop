#include "monad/monad.h"
#include "monad/io.h"
#include "monad/tcp.h"

#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <err.h>


#define CHUNK_SIZE  1024
#define WORKING 9999
static volatile int status = WORKING;
static struct sigaction old_action;


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



static void finish(MonadContext *ctx, struct conn *c, 
        const char *reason) {
    printf("Terminating TCP Server\n");
    if (reason != NULL) {
        perror(reason);
        status = EXIT_FAILURE;
    }
    status = EXIT_SUCCESS;
}


static void client_connected(MonadContext *ctx, struct conn *c, 
        const char *reason) {
    struct sockaddr_in addr = c->addr;
    printf("%s:%d Connected.\n", inet_ntoa(addr.sin_addr), addr.sin_port);
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
    catch_signal();
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
    
    if (monad_tcp_runserver(&bindinfo, finish, &status)) {
        status = EXIT_FAILURE;
    }
    monad_free(bindinfo.client_monad);
    monad_io_deinit();
    return status;
}
