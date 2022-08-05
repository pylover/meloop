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
    struct sockaddr_in addr;
};


typedef void (*conn_event) (struct circuit*, struct tcpserver*, int fd, 
        struct sockaddr *);


struct tcpserver {
    /* io params */
    struct io;

    const char *host;
    int port;
    struct sockaddr_in bind;
    int backlog;
    
    conn_event client_connected;
};


void 
listenA(struct circuit *c, struct tcpserver *s, union any data);


void 
acceptA(struct circuit *c, struct tcpserver *s, union any data);


#endif
