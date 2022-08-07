#ifndef MELOOP_TCP_H
#define MELOOP_TCP_H


#include <meloop/io.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 


struct tcpserverS;
struct tcpclientS;


struct connS {
    struct ioS;
    struct tcpserverS *server;
    struct sockaddr addr;
};


typedef void (*meloop_tcpserver_conn_event) (struct circuitS*, 
        struct tcpserverS*, int fd, struct sockaddr *);


typedef void (*meloop_tcpclient_conn_event) (struct circuitS*, 
        struct tcpclientS*, struct sockaddr *);


struct tcpserverS {
    struct ioS;
    struct sockaddr bind;
    int backlog;
    meloop_tcpserver_conn_event client_connected;
};


struct tcpclientS {
    struct ioS;
    const char *hostname;
    const char *port;
    struct sockaddr hostaddr;
    meloop_tcpclient_conn_event connected;
    int status;
};


void 
listenA(struct circuitS *c, struct tcpserverS *s, union any data);


void 
acceptA(struct circuitS *c, struct tcpserverS *s, union any data);


void 
connectA(struct circuitS *c, struct tcpclientS *s, union any data);


#endif
