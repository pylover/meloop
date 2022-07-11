#include "monad.h"
#include "monad_io.h"

#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define CHUNK_SIZE  1024
#define WORKING 9999
#define ERROR -1
#define OK 0
static volatile int status = WORKING;


// TODO: SOCK_NONBLOCK
// TODO: EPOLLET
// TODO: EAGAIN


struct bind {
    const char *host;
    const int port;
    const int backlog;
    struct sockaddr_in addr;
};


// void listenM(MonadContext *ctx, struct device *dev, struct bind *info) {
//     int fd;
//     int option = 1;
//     int err;
//     struct sockaddr_in *addr = &(info->addr);
// 
//     /* Create socket */
//     fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
// 
//     /* Allow reuse the address */
//     setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
// 
//     /* parse listen address */
//     memset(addr, '0', sizeof(&addr));
//     addr->sin_family = AF_INET;
//     if (info->host == NULL) {
//         addr->sin_addr.s_addr = htonl(INADDR_ANY);
//     } 
//     else if(inet_pton(AF_INET, info->host, &addr->sin_addr) <= 0 ) {
//         monad_failed(ctx, "inet_pton");
//         return;
//     }
//     addr->sin_port = htons(info->port); 
// 
//     /* Bind to tcp port */
//     err = bind(fd, (struct sockaddr*)addr, sizeof(*addr)); 
//     if (err) {
//         monad_failed(ctx, "Cannot bind on: %s", inet_ntoa(addr->sin_addr));
//         return;
//     }
//     
//     /* Listen */
//     err = listen(fd, info->backlog); 
//     if (err) {
//         monad_failed(ctx, "Cannot listen on: %s", inet_ntoa(addr->sin_addr));
//         return;
//     }
//     dev->fd = fd;
//     monad_succeeded(ctx, info);
// }
// 
//     
// void failed(MonadContext *ctx, const char *reason) {
//     if (reason != NULL) {
//         perror(reason);
//         status = ERROR;
//     }
//     
//     status = OK;
// }
// 
// 
// Monad * create_readWriteM(struct device *dev) {
//     Monad *echo = MONAD_RETURN(      mio_waitr, &dev);
//                   MONAD_APPEND(echo, mio_read,  &dev);
//                   MONAD_APPEND(echo, mio_waitw, &dev);
//                   MONAD_APPEND(echo, mio_write, &dev);
//    
//     monad_loop(echo);
//     return echo;
// }
// 
// 
// void acceptM(MonadContext *ctx, struct device *dev, struct bind *info) {
// 	int fd;
// 	socklen_t addrlen = sizeof(struct sockaddr);
// 	struct sockaddr addr; 
// 
// 	fd = accept(dev->fd, &addr, &addrlen);
// 	if (fd == -1) {
//         monad_failed(ctx, "accept");
//         return;
// 	}
//     
//     // printf("Client connected: %s\n", inet_ntoa(addr.sin_addr));
//     // // TODO: free
//     // struct device *cdev = malloc(sizeof(struct device));
//     // if (cdev == NULL) {
//     //     monad_failed(ctx, "malloc");
//     //     return;
//     // }
//     // Monad * client_monad = createReadWriteM(cdev);
//     // io_run(
// 
//     monad_succeeded(ctx, info);
// }
// 
// 
// Monad * create_listenAcceptM(struct device * dev) {
//     Monad *listen_ = MONAD_RETURN(         listenM,   &dev);
//     Monad *accept_ = MONAD_RETURN(         mio_waitr, &dev);
//                      MONAD_APPEND(accept_, acceptM,   &dev);
//     
//     monad_loop(accept_);
//     monad_bind(listen_, accept_);
//     return listen_;
// }
// 
// 
// int main() {
//     mio_init(0);
//     
//     struct bind bindinfo = {
//         .host = "127.0.0.1",
//         .port = 9090,
//         .backlog = 2
//     };
//     struct packet p = {0, malloc(CHUNK_SIZE)};
//     struct device listendev = {-1, CHUNK_SIZE};
//     Monad *listen_monad = create_listenAcceptM(&listendev);
// 
//     if (mio_run(listen_monad, &bindinfo, NULL, failed)) {
//         err(1, "mio_run");
//     }
//    
//     if (p.data != NULL) {
//         free(p.data);
//     }
//     monad_free(listen_monad);
//     mio_deinit();
//     return status;
// }
void main () {}
