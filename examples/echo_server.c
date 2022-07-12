#include "monad.h"
#include "monad_io.h"
#include "monad_tcp.h"

#include <stdlib.h>
#include <err.h>


#define CHUNK_SIZE  1024
#define WORKING 9999
#define ERROR -1
#define OK 0
static volatile int status = WORKING;
static struct device clientdev = {false, CHUNK_SIZE};
static Monad * client_monad;


// TODO: SOCK_NONBLOCK
// TODO: EPOLLET
// TODO: EAGAIN


void finish(MonadContext *ctx, struct conn *c, const char *reason) {
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


Monad * create_listenAcceptM(struct device * dev) {
    Monad *listen_ = MONAD_RETURN(         listenM,   &dev);
    Monad *accept_ = MONAD_RETURN(         mio_waitr, &dev);
                     MONAD_APPEND(accept_, acceptM,   &dev);
    
    monad_loop(accept_);
    monad_bind(listen_, accept_);
    return listen_;
}


Monad * create_clientM(struct device *dev) {
    Monad *echo = MONAD_RETURN(      mio_waitr, &dev);
                  MONAD_APPEND(echo, mio_read,  &dev);
                  MONAD_APPEND(echo, mio_waitw, &dev);
                  MONAD_APPEND(echo, mio_write, &dev);
   
    monad_loop(echo);
    return echo;
}


int main() {
    mio_init(0);
    
    struct bind bindinfo = {
        .host = "127.0.0.1",
        .port = 9090,
        .backlog = 2,
        .client_monad = create_clientM(&clientdev),
        .client_connected = (monad_finish) client_connected,
        .client_closed = (monad_finish) client_closed
    };

    struct conn listenc = {
        .ptr = &bindinfo
    };
    struct device listendev = {false, 0};
    Monad *listen_monad = create_listenAcceptM(&listendev);

    if (MIO_RUN(listen_monad, &listenc, finish)) {
        err(1, "mio_run");
    }
    
    monad_free(client_monad);
    monad_free(listen_monad);
    mio_deinit();
    return status;
}
