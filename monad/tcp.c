#include "io.h"
#include "tcp.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/epoll.h>


#define TCP_BACKLOG_DEFAULT 2


static void _client_free(struct conn *c) {
    if (c->data != NULL) {
        free(c->data);
    }
    
    free(c);
}


void listenM(MonadContext *ctx, struct io_props *dev, struct conn *c) {
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


static void client_closed(MonadContext *ctx, struct conn *c, 
        const char *reason) {
    struct bind *info = (struct bind*) c->ptr;
    
    close(c->rfd);
    close(c->wfd);

    if (info->client_closed != NULL) {
        info->client_closed(ctx, c, reason);
    }
    _client_free(c);
}


void acceptM(MonadContext *ctx, struct io_props *dev, struct conn *c) {
	int fd;
	socklen_t addrlen = sizeof(struct sockaddr);
    struct bind *info = (struct bind*) c->ptr;
	struct sockaddr addr; 

	fd = accept4(c->rfd, &addr, &addrlen, SOCK_NONBLOCK);
	if (fd == -1) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            monad_again(ctx, dev, c, EPOLLIN);
        }
        else {
            monad_failed(ctx, c, "accept4");
        }
        return;
	}
    
    // TODO: printf("Client connected: %s\n", inet_ntoa(addr.sin_addr));
    /* Will be free at tcp: client_free() */
    struct conn *cc = malloc(sizeof(struct conn));
    if (cc == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }

    cc->rfd = fd; 
    cc->wfd = fd; 
    cc->size = 0; 

    /* Will be free at tcp: client_free() */
    cc->data = malloc(dev->readsize);
    if (cc->data == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }
    cc->ptr = info;

    if (info->client_connected != NULL) {
        info->client_connected(ctx, cc, NULL);
    }
    MONAD_RUNALL(info->client_monad, cc, client_closed);
    monad_succeeded(ctx, c);
}


void monad_tcp_runserver(struct bind *info, monad_tcp_finish finish) {
    struct conn listenc = {
        .ptr = info
    };
    struct io_props listen_props = {false, 0};
   
    /* Default backlog if not given. */
    if (info->backlog == 0) {
        info->backlog = TCP_BACKLOG_DEFAULT;
    }

    /* Create an open monad chain for listen */
    Monad *listen_m = MONAD_RETURN(          listenM,   &listen_props);

    /* Create a closed monad chain for accept a connection and wait for the
       next clinet. */
    Monad *accept_m = MONAD_RETURN(acceptM, &listen_props);
    monad_loop(accept_m);
    
    /* Bind them together */
    monad_bind(listen_m, accept_m);

    /* Run it within IO context. */
    if (MONAD_IO_RUN(listen_m, &listenc, (monad_finish)finish)) {
        err(1, "monad_io_run");
    }
   
    /* Dispose */
    monad_free(listen_m);
}
