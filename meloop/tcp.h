#ifndef MELOOP_TCP_H
#define MELOOP_TCP_H


#include <meloop/io.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 


struct tcpserverS;
struct tcpclientS;


typedef void (*meloop_tcpserver_conn_event) (struct circuitS*, 
        struct fileS*, int fd, struct sockaddr *);


typedef void (*meloop_tcpclient_conn_event) (struct circuitS*, 
        struct fileS*, struct sockaddr *);


struct tcpconnS {
    struct fileS;
    struct sockaddr addr;
};


struct tcpserverS {
    struct sockaddr bind;
    int backlog;
    meloop_tcpserver_conn_event client_connected;
};


struct tcpclientS {
    const char *hostname;
    const char *port;
    struct sockaddr hostaddr;
    meloop_tcpclient_conn_event connected;
    int status;
};


void 
listenA(struct circuitS *c, struct fileS *s, union any data);


void 
acceptA(struct circuitS *c, struct fileS *s, union any data);


void 
connectA(struct circuitS *c, struct fileS *s, union any data);


#endif
