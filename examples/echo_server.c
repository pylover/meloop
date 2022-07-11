#include "monad.h"
#include "monad_io.h"

#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define CHUNK_SIZE  1024
#define WORKING 9999
#define ERROR -1
#define OK 0
static volatile int status = WORKING;
static Monad * client_monad;


// TODO: SOCK_NONBLOCK
// TODO: EPOLLET
// TODO: EAGAIN


struct bind {
    const char *host;
    const int port;
    const int backlog;
    struct sockaddr_in addr;
};


void listenM(MonadContext *ctx, struct device *dev, struct conn *c) {
    int fd;
    int option = 1;
    int err;
    struct bind *info = (struct bind*) c->ptr;
    struct sockaddr_in *addr = &(info->addr);

    /* Create socket */
    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    /* Allow reuse the address */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* parse listen address */
    memset(addr, '0', sizeof(&addr));
    addr->sin_family = AF_INET;
    if (info->host == NULL) {
        addr->sin_addr.s_addr = htonl(INADDR_ANY);
    } 
    else if(inet_pton(AF_INET, info->host, &addr->sin_addr) <= 0 ) {
        monad_failed(ctx, c, "inet_pton");
        return;
    }
    addr->sin_port = htons(info->port); 

    /* Bind to tcp port */
    err = bind(fd, (struct sockaddr*)addr, sizeof(*addr)); 
    if (err) {
        monad_failed(ctx, c, "Cannot bind on: %s", inet_ntoa(addr->sin_addr));
        return;
    }
    
    /* Listen */
    err = listen(fd, info->backlog); 
    if (err) {
        monad_failed(ctx, c, "Cannot listen on: %s", 
                inet_ntoa(addr->sin_addr));
        return;
    }
    c->rfd = fd;
    monad_succeeded(ctx, c);
}

    
void finish(MonadContext *ctx, struct conn *c, const char *reason) {
    if (reason != NULL) {
        perror(reason);
        status = ERROR;
    }
    status = OK;
}


Monad * create_readWriteM(struct device *dev) {
    Monad *echo = MONAD_RETURN(      mio_waitr, &dev);
                  MONAD_APPEND(echo, mio_read,  &dev);
                  MONAD_APPEND(echo, mio_waitw, &dev);
                  MONAD_APPEND(echo, mio_write, &dev);
   
    monad_loop(echo);
    return echo;
}


static void client_free(struct conn *c) {
    if (c->data != NULL) {
        free(c->data);
    }
}


static void client_closed(MonadContext *ctx, struct conn *c, 
        const char *reason) {
    if (reason == NULL) {
        printf("client error: %s\n", reason);
    }
    else {
        printf("client closed: %d\n", c->rfd);
    }
    client_free(c);
}


void acceptM(MonadContext *ctx, struct device *dev, struct conn *c) {
	int fd;
	socklen_t addrlen = sizeof(struct sockaddr);
	struct sockaddr addr; 

	fd = accept(c->rfd, &addr, &addrlen);
	if (fd == -1) {
        monad_failed(ctx, c, "accept");
        return;
	}
    
    // printf("Client connected: %s\n", inet_ntoa(addr.sin_addr));
    printf("Client connected: %d\n", fd);

    // TODO: free
    struct conn *cc = malloc(sizeof(struct conn));
    if (cc == NULL) {
        monad_failed(ctx, c, "malloc");
        return;
    }
    cc->rfd = fd, 
    cc->wfd = fd, 
    cc->size = 0, 
    cc->data = malloc(CHUNK_SIZE), 

    MONAD_RUN(client_monad, cc, client_closed);
    monad_succeeded(ctx, c);
}


Monad * create_listenAcceptM(struct device * dev) {
    Monad *listen_ = MONAD_RETURN(         listenM,   &dev);
    Monad *accept_ = MONAD_RETURN(         mio_waitr, &dev);
                     MONAD_APPEND(accept_, acceptM,   &dev);
    
    monad_loop(accept_);
    monad_bind(listen_, accept_);
    return listen_;
}


int main() {
    mio_init(0);
    
    struct bind bindinfo = {
        .host = "127.0.0.1",
        .port = 9090,
        .backlog = 2
    };

    struct device clientdev = {false, CHUNK_SIZE};
    client_monad = create_readWriteM(&clientdev);

    struct conn listenc = {
        .rfd = -1, 
        .wfd = -1, 
        .size = 0, 
        .data = NULL, 
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
