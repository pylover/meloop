#include "monad.h"
#include "io.h"
#include "tcp.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>


void listenM(MonadContext *ctx, struct device *dev, struct conn *c) {
    int fd;
    int option = 1;
    int res;
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
    res = bind(fd, (struct sockaddr*)addr, sizeof(*addr)); 
    if (res) {
        monad_failed(ctx, c, "Cannot bind on: %s", inet_ntoa(addr->sin_addr));
        return;
    }
    
    /* Listen */
    res = listen(fd, info->backlog); 
    if (res) {
        monad_failed(ctx, c, "Cannot listen on: %s", 
                inet_ntoa(addr->sin_addr));
        return;
    }
    c->rfd = fd;
    monad_succeeded(ctx, c);
}

    
static void client_free(struct conn *c) {
    if (c->data != NULL) {
        free(c->data);
    }
}


static void client_closed(MonadContext *ctx, struct conn *c, 
        const char *reason) {
    struct bind *info = (struct bind*) c->ptr;
    
    close(c->rfd);
    close(c->wfd);
    info->client_closed(ctx, c, reason);
    client_free(c);
}


void acceptM(MonadContext *ctx, struct device *dev, struct conn *c) {
	int fd;
	socklen_t addrlen = sizeof(struct sockaddr);
    struct bind *info = (struct bind*) c->ptr;
	struct sockaddr addr; 

	fd = accept(c->rfd, &addr, &addrlen);
	if (fd == -1) {
        monad_failed(ctx, c, "accept");
        return;
	}
    
    // TODO: printf("Client connected: %s\n", inet_ntoa(addr.sin_addr));
    // TODO: free
    struct conn *cc = malloc(sizeof(struct conn));

    if (cc == NULL) {
        monad_failed(ctx, c, "malloc");
        return;
    }
    cc->rfd = fd, 
    cc->wfd = fd, 
    cc->size = 0, 
    cc->data = malloc(dev->readsize), 
    cc->ptr = info;

    info->client_connected(ctx, cc, NULL);
    MONAD_RUN(info->client_monad, cc, client_closed);
    monad_succeeded(ctx, c);
}
