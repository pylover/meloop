#include "meloop/arrow.h"
#include "meloop/addr.h"
#include "meloop/io.h"
#include "meloop/tcp.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/epoll.h>


void 
listenA(struct circuitS *c, struct tcpserverS *s, union any data) {
    int fd;
    int option = 1;
    int res;
    struct sockaddr *addr = &(s->bind);
    
    /* Create socket */
    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    /* Allow reuse the address */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* Bind to tcp port */
    res = bind(fd, &(s->bind), sizeof(s->bind)); 
    if (res) {
        ERROR_A(c, s, data, "Cannot bind on: %s", meloop_addr_dump(addr));
        return;
    }
    
    /* Listen */
    res = listen(fd, s->backlog); 
    if (res) {
        ERROR_A(c, s, data, "Cannot listen on: %s", meloop_addr_dump(addr));
        return;
    }
    s->rfd = fd;
    s->wfd = fd;
    RETURN_A(c, s, NULL);
}


void 
acceptA(struct circuitS *c, struct tcpserverS *s, union any data) {
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
