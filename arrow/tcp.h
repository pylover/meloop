#ifndef ARROW_TCP_H
#define ARROW_TCP_H


#include <arrow/io.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 


struct tcpserver {
    /* io params */
    struct io;

    const char *host;
    int port;
    struct sockaddr_in bind;
    int backlog;
};


void 
listenA(struct circuit *c, struct tcpserver *s);


void 
acceptA(struct circuit *c, struct tcpserver *s, union any data);


#endif
