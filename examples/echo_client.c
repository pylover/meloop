#include "monad/tcp.h"
#include "monad/random.h"

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
    printf("Terminating TCP Connection\n");
    if (reason != NULL) {
        perror(reason);
        status = EXIT_FAILURE;
    }
    status = EXIT_SUCCESS;
}


static void connected(MonadContext *ctx, struct conn *c, 
        const char *reason) {
    struct sockaddr_in addr = c->addr;
    printf("%s:%d Connected.\n", inet_ntoa(addr.sin_addr), addr.sin_port);
}


static void disconnected(MonadContext *ctx, struct conn *c, 
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
    
    static struct rand_props randprops = {
        .epollflags = EPOLLET,
        .readsize = CHUNK_SIZE
    };
    static struct io_props ioprops = {
        .epollflags = EPOLLET, 
        .readsize = CHUNK_SIZE
    };
    
    Monad *init = MONAD_RETURN(      urandom_openM, &randprops);

    // Monad *init = MONAD_RETURN(      urandom_openM, &props);
    //               MONAD_APPEND(init, connectM,      &props);
    // Monad *loop = MONAD_RETURN(      urandomM,      &props);
    //               MONAD_APPEND(loop, writerM,       &props);
    //               MONAD_APPEND(loop, readerM,       &props);
    //               MONAD_APPEND(loop, printM,        &props);
    // monad_loop(loop);
    // monad_bind(init, loop);

    struct connect connectinfo = {
        .host = "127.0.0.1",
        .port = 9090,
        .worker = init,
        .connected = connected,
        .disconnected = disconnected
    };
    
    // monad_run(init, connectinfo, 
    if (monad_tcp_connect(&connectinfo, finish, &status)) {
        status = EXIT_FAILURE;
    }
    monad_free(connectinfo.worker);
    monad_io_deinit();
    return status;
}
