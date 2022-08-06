#ifndef MELOOP_TCP_H
#define MELOOP_TCP_H


#include <meloop/io.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 


struct tcpserverS;


struct connS {
    struct ioS;
    struct tcpserverS *server;
    struct sockaddr addr;
};


typedef void (*conn_event) (struct circuitS*, struct tcpserverS*, int fd, 
        struct sockaddr *);


struct tcpserverS {
    struct ioS;
    struct sockaddr bind;
    int backlog;
    conn_event client_connected;
};


void 
listenA(struct circuitS *c, struct tcpserverS *s, union any data);


void 
acceptA(struct circuitS *c, struct tcpserverS *s, union any data);


#endif
