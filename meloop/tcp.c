#include "meloop/arrow.h"
#include "meloop/addr.h"
#include "meloop/io.h"
#include "meloop/tcp.h"
#include "meloop/ev.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>


void 
listenA(struct circuitS *c, struct fileS *s, union any data) {
    struct tcpserverS *priv = meloop_priv_ptr(c);
    int fd;
    int option = 1;
    int res;
    struct sockaddr *addr = &(priv->bind);
    
    /* Create socket */
    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    /* Allow reuse the address */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* Bind to tcp port */
    res = bind(fd, addr, sizeof(priv->bind)); 
    if (res) {
        ERROR_A(c, s, data, "Cannot bind on: %s", meloop_addr_dump(addr));
        return;
    }
    
    /* Listen */
    res = listen(fd, priv->backlog); 
    if (res) {
        ERROR_A(c, s, data, "Cannot listen on: %s", meloop_addr_dump(addr));
        return;
    }
    s->fd = fd;
    RETURN_A(c, s, NULL);
}


void 
acceptA(struct circuitS *c, struct fileS *s, union any data) {
    struct tcpserverS *priv = meloop_priv_ptr(c);
    int fd;
    socklen_t addrlen = sizeof(struct sockaddr);
    struct sockaddr addr; 

    fd = accept4(s->fd, &addr, &addrlen, SOCK_NONBLOCK);
    if (fd == -1) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, data, s->fd, EPOLLIN);
        }
        else {
            ERROR_A(c, s, data, "accept4");
        }
        return;
    }
   
    if (priv->client_connected != NULL) {
        priv->client_connected(c, s, fd, &addr);
    }
    RETURN_A(c, s, data);
}


#define _IDLE       1
#define _CONNECTING 2
#define _CONNECTED  3
#define _FAILED     4


static void
_connect_continue(struct circuitS *c, struct fileS *s, union any data) {
    /* After epoll(2) indicates writability, use getsockopt(2) to read the
       SO_ERROR option at level SOL_SOCKET to determine whether connect()
       completed successfully (SO_ERROR is zero) or unsuccessfully
       (SO_ERROR is one of the usual error codes listed here,
       explaining the reason for the failure).
    */
    struct tcpclientS *priv = meloop_priv_ptr(c);
    int err;
    int l = 4;
    if (getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &err, &l) != OK) {
        priv->status = _FAILED;
        ERROR_A(c, s, data, "getsockopt");
        return;
    }
    if (err != OK) {
        priv->status = _FAILED;
        errno = err;
        ERROR_A(c, s, data, "TCP connect");
        meloop_ev_dearm(s->fd);
        close(s->fd);
        return;
    }

    /* Hooray, Connected! */
    priv->status = _CONNECTED;
    RETURN_A(c, s, data);
}


void 
connectA(struct circuitS *c, struct fileS *s, union any data) {
    struct tcpclientS *priv = meloop_priv_ptr(c);
    if (priv->status == _CONNECTING) {
        _connect_continue(c, s, data);
        return;
    }

    int fd;
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *try;

    /* Resolve hostname */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;         /* Allow IPv4 only.*/
    // TODO: IPv6
    // hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_protocol = 0;
    if (getaddrinfo(priv->hostname, priv->port, &hints, &result) != 0) {
        ERROR_A(c, s, data, "Name resolution failed.");
        return;
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address.
    */
    for (try = result; try != NULL; try = try->ai_next) {
        fd = socket(
                try->ai_family,
                try->ai_socktype | SOCK_NONBLOCK,
                try->ai_protocol
            );
        if (fd < 0) {
            continue;
        }

        if (connect(fd, try->ai_addr, try->ai_addrlen) == OK) {
            /* Connection success */
            break;
        }

        if (errno == EINPROGRESS) {
            /* Waiting to connect */
            break;
        }

        close(fd);
        fd = -1;
    }

    if (fd < 0) {
        freeaddrinfo(result);
        ERROR_A(c, s, data, "TCP connect");
        return;
    }

    /* Connection success */
    /* Update state */
    memcpy(&(priv->hostaddr), try->ai_addr, sizeof(struct sockaddr));
    s->fd = fd;
    freeaddrinfo(result);

    if (errno == EINPROGRESS) {
        /* Waiting to connect */
        /* The socket is nonblocking and the connection cannot be
           completed immediately. It is possible to epoll(2) for
           completion by selecting the socket for writing.
        */
        priv->status = _CONNECTING;
        WAIT_A(c, s, data, s->fd, EPOLLOUT);
        return;
    }

    /* Seems everything is ok. */
    priv->status = _CONNECTED;
    RETURN_A(c, s, data);
}
