#ifndef MONAD_TCP_H
#define MONAD_TCP_H


#include <monad/io.h>

#include <arpa/inet.h>


typedef void (*monad_tcp_finish) (MonadContext*, struct conn *,
        const char *reason);


struct bind {
    struct conn;
    const char *host;
    const int port;
    int backlog;
    Monad * worker;
    monad_tcp_finish client_connected;
    monad_tcp_finish client_closed;
};


struct connect {
    struct conn;
    const char *host;
    const int port;
    Monad * worker;
    monad_tcp_finish connected;
    monad_tcp_finish disconnected;
};


int 
monad_tcp_runserver(struct bind *info, monad_tcp_finish finish, 
        volatile int *status);

/* TCP Server Monads */
void 
listenM(MonadContext *ctx, struct io_props *dev, struct bind *c);

void 
acceptM(MonadContext *ctx, struct io_props *dev, struct bind *c);


#endif
