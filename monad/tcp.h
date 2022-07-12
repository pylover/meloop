#ifndef MONAD_TCP_H
#define MONAD_TCP_H


#include <arpa/inet.h>


struct bind {
    const char *host;
    const int port;
    const int backlog;
    struct sockaddr_in addr;
    Monad * client_monad;
    monad_finish client_connected;
    monad_finish client_closed;
};


void listenM(MonadContext *ctx, struct device *dev, struct conn *c);
void acceptM(MonadContext *ctx, struct device *dev, struct conn *c);


#endif
