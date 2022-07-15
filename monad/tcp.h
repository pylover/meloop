#ifndef MONAD_TCP_H
#define MONAD_TCP_H


#include <monad/io.h>

#include <arpa/inet.h>


typedef void (*monad_tcp_finish) (MonadContext*, struct conn *,
        const char *reason);


struct bind {
    const char *host;
    const int port;
    int backlog;
    struct sockaddr_in addr;
    Monad * client_monad;
    monad_tcp_finish client_connected;
    monad_tcp_finish client_closed;
};


int monad_tcp_runserver(struct bind *info, monad_tcp_finish finish, 
        volatile int *status);

/* TCP Server Monads */
void listenM(MonadContext *ctx, struct io_props *dev, struct conn *c);
void acceptM(MonadContext *ctx, struct io_props *dev, struct conn *c);


#endif
