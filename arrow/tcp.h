#ifndef ARROW_TCP_H
#define ARROW_TCP_H


#include <arpa/inet.h>


struct tcpserver {
    const char *host;
    int port;
    struct sockaddr_in bind;
    int backlog;
    int listenfd;
    int epollflags;
};


void 
listenA(struct circuit *c, struct tcpserver *s);


#endif
