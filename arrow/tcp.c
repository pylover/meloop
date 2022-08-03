#include "arrow/arrow.h"
#include "arrow/tcp.h"


void 
listenA(struct circuit *c, struct tcpserver *s) {
    int fd;
    int option = 1;
    int res;
    struct sockaddr_in *addr = &(s->bind);
    
    /* Create socket */
    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    /* Allow reuse the address */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* parse listen address */
    memset(addr, '0', sizeof(&addr));
    addr->sin_family = AF_INET;
    if (s->host == NULL) {
        addr->sin_addr.s_addr = htonl(INADDR_ANY);
    } 
    else if(inet_pton(AF_INET, s->host, &addr->sin_addr) <= 0 ) {
        errorA(c, s, "inet_pton");
        return;
    }
    addr->sin_port = htons(s->port); 

    /* Bind to tcp port */
    res = bind(fd, (struct sockaddr*)addr, sizeof(*addr)); 
    if (res) {
        errorA(c, s, "Cannot bind on: %s", inet_ntoa(addr->sin_addr));
        return;
    }
    
    /* Listen */
    res = listen(fd, s->backlog); 
    if (res) {
        errorA(c, s, "Cannot listen on: %s", inet_ntoa(addr->sin_addr));
        return;
    }
    s->listenfd = fd;
    RETURN_A(c, s, NULL);
}
