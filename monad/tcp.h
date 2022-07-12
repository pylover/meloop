#ifndef MONAD_TCP_H
#define MONAD_TCP_H


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



void listenM(MonadContext *ctx, struct device *dev, struct conn *c);
void acceptM(MonadContext *ctx, struct device *dev, struct conn *c);
void monad_tcp_runserver(struct bind *info, monad_tcp_finish finish);


#endif
