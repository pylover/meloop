#include "tcp.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/epoll.h>


#define TCP_BACKLOG_DEFAULT 2


static void 
_client_free(struct conn *c) {
    if (c->data != NULL) {
        free(c->data);
    }
    
    free(c);
}


void 
listenM(MonadContext *ctx, struct io_props *dev, struct bind *c) {
    int fd;
    int option = 1;
    int res;
    struct sockaddr_in *addr = &(c->addr);
    
    /* Create socket */
    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    /* Allow reuse the address */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* parse listen address */
    memset(addr, '0', sizeof(&addr));
    addr->sin_family = AF_INET;
    if (c->host == NULL) {
        addr->sin_addr.s_addr = htonl(INADDR_ANY);
    } 
    else if(inet_pton(AF_INET, c->host, &addr->sin_addr) <= 0 ) {
        monad_failed(ctx, c, "inet_pton");
        return;
    }
    addr->sin_port = htons(c->port); 

    /* Bind to tcp port */
    res = bind(fd, (struct sockaddr*)addr, sizeof(*addr)); 
    if (res) {
        monad_failed(ctx, c, "Cannot bind on: %s", inet_ntoa(addr->sin_addr));
        return;
    }
    
    /* Listen */
    res = listen(fd, c->backlog); 
    if (res) {
        monad_failed(ctx, c, "Cannot listen on: %s", 
                inet_ntoa(addr->sin_addr));
        return;
    }
    c->rfd = fd;
    c->wfd = fd;
    monad_succeeded(ctx, c);
}


static void 
client_closed(MonadContext *ctx, struct conn *c, const char *reason) {
    struct bind *info = (struct bind*) c->ptr;

    close(c->rfd);
    close(c->wfd);

    if (info->client_closed != NULL) {
        info->client_closed(ctx, c, reason);
    }
    _client_free(c);
}


// void connectM(MonadContext *ctx, struct io_props *dev, struct conn *c) {
// 
// }


// err_t tcp_connect (tcp **tp, const char *hostname, const char *port, 
//         tcp_callback cb, void *ptr) {
//     int fd;
//     struct addrinfo hints;
//     struct addrinfo *result;
//     struct addrinfo *try;
//     tcp *t; 
//     
//     DEBUG("%s %s", hostname, port);
//     //* Allocate memory */
//     *tp = t = malloc(sizeof(struct tcp));
// 
//     t->fd = -1;
//     t->callback = cb;
//     t->ptr = ptr;
// 
//     /* Resolve hostname */
//     memset(&hints, 0, sizeof(hints));
//     hints.ai_family = AF_INET;         /* Allow IPv4 only.*/
//     // TODO: IPv6
//     // hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
//     hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
//     hints.ai_flags = AI_NUMERICSERV;
//     hints.ai_protocol = 0;
//     if (getaddrinfo(hostname, port, &hints, &result) != 0) {
//         free(t);
//         FATAL("Name resolution failed.");
//     }
// 
//     /* getaddrinfo() returns a list of address structures.
//        Try each address until we successfully connect(2).
//        If socket(2) (or connect(2)) fails, we (close the socket
//        and) try the next address. 
//     */
//     for (try = result; try != NULL; try = try->ai_next) {
//         fd = socket(
//                 try->ai_family, 
//                 try->ai_socktype | SOCK_NONBLOCK, 
//                 try->ai_protocol
//             );
//         if (fd < 0) {
//             continue;
//         }
// 
//         if (connect(fd, try->ai_addr, try->ai_addrlen) == OK) {
//             /* Connection sucess */
//             /* Register websocket for write */
//             if (ev_arm(fd, EV_WRITE, (ev_callback)_tcp_ready, t) != OK) {
//                 free(t);
//                 FATAL("ev_arm: ADD");
//             }
//             break;
//         }
//         
//         if (errno == EINPROGRESS) {
//             /* Waiting to connect */
//             /* The socket is nonblocking and the connection cannot be 
//                completed immediately. It is possible to epoll(2) for
//                completion by selecting the socket for writing. 
//             */
//             if (ev_arm(fd, EV_WRITE, (ev_callback)_connect_continue, 
//                         t) != OK) {
//                 free(t);
//                 FATAL("ev_arm");
//             }
//             break;
//         }
// 
//         close(fd);
//         fd = -1;
//     }
// 
//     /* No longer needed */
//     freeaddrinfo(result);
//    
//     if (fd < 0) {
//         ERROR("TCP connect");
//         free(t);
//         return ERR;
//     }
//     else {
//         /* Update state */
//         t->fd = fd;
// 
//         /* Seems everything is ok. */
//         return OK;
//     }
// }
// static void _connect_continue (tcp *t, enum ev_event event) {
//     /* After epoll(2) indicates writability, use getsockopt(2) to read the 
//        SO_ERROR option at level SOL_SOCKET to determine whether connect() 
//        completed successfully (SO_ERROR is zero) or unsuccessfully 
//        (SO_ERROR is one of the usual error codes listed here, 
//        explaining the reason for the failure).
//     */
//     int err;
//     int l = 4;
//     if (getsockopt(t->fd, SOL_SOCKET, SO_ERROR, 
//             &err, &l) != OK) {
//         FATAL("getsockopt");
//     }
//     if (err != OK) {
//         errno = err;
//         FATAL("TCP connect");
//         ev_dearm(t->fd);
//         close(t->fd);
//     }
//     else {
//         /* Hooray, Connected! */
//         DEBUG ("Connected");
//         if (ev_rearm(t->fd, EV_WRITE, (ev_callback)_tcp_ready, t) != OK) {
//             FATAL("ev_rearm");
//         }
//     }
// }
// 
// 


void 
acceptM(MonadContext *ctx, struct io_props *dev, struct bind *c) {
	int fd;
	socklen_t addrlen = sizeof(struct sockaddr);
	struct sockaddr addr; 

	fd = accept4(c->rfd, &addr, &addrlen, SOCK_NONBLOCK);
	if (fd == -1) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            MONAD_IO_AGAIN(ctx, dev, c, EPOLLIN);
        }
        else {
            monad_failed(ctx, c, "accept4");
        }
        return;
	}
   
    /* Will be free at tcp: client_free() */
    struct conn *cc = malloc(sizeof(struct conn));
    if (cc == NULL) {
        close(fd);
        monad_failed(ctx, c, "Out of memory");
        return;
    }

    cc->rfd = fd; 
    cc->wfd = fd; 
    cc->size = 0; 
    memcpy(&(cc->addr), &addr, sizeof(struct sockaddr_in));

    /* Will be free at tcp.c: client_free() */
    struct io_props *cdev = monad_args(c->worker);
    cc->data = malloc(cdev->readsize);
    if (cc->data == NULL) {
        close(fd);
        free(cc);
        monad_failed(ctx, c, "Out of memory");
        return;
    }
    cc->ptr = c;

    if (c->client_connected != NULL) {
        c->client_connected(ctx, cc, NULL);
    }
    MONAD_RUN(c->worker, cc, client_closed);
    monad_succeeded(ctx, c);
}


Monad * 
tcpserverF() {
    // TODO: maybe enable edge triggering for listen socket
    struct io_props listen_props = {false, 0};
   
    /* Create an open monad chain for listen */
    Monad *listen_m = MONAD_RETURN(listenM, &listen_props);

    /* Create a closed monad chain for accept a connection and wait for the
       next clinet. */
    Monad *accept_m = MONAD_RETURN(acceptM, &listen_props);
    monad_loop(accept_m);
    
    /* Bind them together */
    monad_bind(listen_m, accept_m);
    
    return listen_m;
}


int 
monad_tcp_runserver(struct bind *info, monad_tcp_finish finish, 
        volatile int *status) {

    /* Default backlog if not given. */
    if (info->backlog == 0) {
        info->backlog = TCP_BACKLOG_DEFAULT;
    }

    /* Create monad */
    struct monad *srv_m = tcpserverF();

    /* Run TCP server monad. */
    struct monad_context * ctx = MONAD_RUN(srv_m, info, finish);

    /* Start and wait for event loop */
    int res = monad_io_loop(status);
    
    /* Dispose */
    if (ctx != NULL) {
        monad_terminate(ctx, info, NULL);
    }
    monad_free(srv_m);
    if (info->garbage != NULL) {
        free(info->garbage);
    }

    /* return result */
    return res;
}
