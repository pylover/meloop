#include "arrow/arrow.h"
#include "arrow/io.h"
#include "arrow/tcp.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/epoll.h>


void 
listenA(struct circuit *c, struct tcpserver *s, union any data) {
    int fd;
    int option = 1;
    int res;
    struct sockaddr_in *addr = &(s->bind);
    
    /* Create socket */
    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    /* Allow reuse the address */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* parse listen address */
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    if (s->host == NULL) {
        addr->sin_addr.s_addr = htonl(INADDR_ANY);
    } 
    else if(inet_pton(AF_INET, s->host, &addr->sin_addr) <= 0 ) {
        ERROR_A(c, s, data, "inet_pton");
        return;
    }
    addr->sin_port = htons(s->port); 

    /* Bind to tcp port */
    res = bind(fd, (struct sockaddr*)addr, sizeof(*addr)); 
    if (res) {
        ERROR_A(c, s, data, "Cannot bind on: %s", inet_ntoa(addr->sin_addr));
        return;
    }
    
    /* Listen */
    res = listen(fd, s->backlog); 
    if (res) {
        ERROR_A(c, s, data, "Cannot listen on: %s", 
                inet_ntoa(addr->sin_addr));
        return;
    }
    s->rfd = fd;
    s->wfd = fd;
    RETURN_A(c, s, NULL);
}


void 
acceptA(struct circuit *c, struct tcpserver *s, union any data) {
    int fd;
    socklen_t addrlen = sizeof(struct sockaddr);
    struct sockaddr addr; 

    fd = accept4(s->rfd, &addr, &addrlen, SOCK_NONBLOCK);
    if (fd == -1) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, data, s->rfd, EPOLLIN);
        }
        else {
            ERROR_A(c, s, data, "accept4");
        }
        return;
    }
   
    if (s->client_connected != NULL) {
        s->client_connected(c, s, fd, &addr);
    }
    returnA(c, s, data);
}
