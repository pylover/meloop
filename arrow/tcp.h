#ifndef ARROW_TCP_H
#define ARROW_TCP_H


#include <arrow/io.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 


struct tcpserver;


struct conn {
    struct io;
    struct tcpserver *server;
    struct sockaddr addr;
};


typedef void (*conn_event) (struct circuit*, struct tcpserver*, int fd, 
        struct sockaddr *);


struct tcpserver {
    struct io;
    struct sockaddr bind;
    int backlog;
    conn_event client_connected;
};


int
parse_sockaddr(struct sockaddr *addr, const char *host, unsigned short port);


void 
listenA(struct circuit *c, struct tcpserver *s, union any data);


void 
acceptA(struct circuit *c, struct tcpserver *s, union any data);


#endif
