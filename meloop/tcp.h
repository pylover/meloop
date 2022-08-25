#ifndef MELOOP_TCP_H
#define MELOOP_TCP_H


#include <meloop/io.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 


struct tcpserverP;
struct tcpclientP;


typedef void (*meloop_tcpserver_conn_event) (struct circuitS*, 
        void *state, int fd, struct sockaddr *);


typedef void (*meloop_tcpclient_conn_event) (struct circuitS*, 
        void *state, struct sockaddr *);


struct tcpconnS {
    struct fileS;
    struct sockaddr addr;
};


struct tcpserverP {
    struct ioP;
    const char *bindaddr;
    unsigned short bindport;
    struct sockaddr bind;
    int backlog;
    meloop_tcpserver_conn_event client_connected;
};


struct tcpclientP {
    struct ioP;
    const char *hostname;
    const char *port;
    struct sockaddr hostaddr;
    meloop_tcpclient_conn_event connected;
    int status;
};


void 
listenA(struct circuitS *c, void *s, struct fileS *f,
        struct tcpserverP *priv);


void 
acceptA(struct circuitS *c, void *s, struct fileS *f,
        struct tcpserverP *priv);


void 
connectA(struct circuitS *c, void *s, struct fileS *f, 
        struct tcpclientP *priv);


#endif
