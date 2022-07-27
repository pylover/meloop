#include "arrow/arrow.h"
#include "arrow/io.h"
#include "arrow/ev.h"

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>


void 
waitA(struct circuit *c, struct conn *conn, union args data, int op) {
    struct bag *bag = malloc(sizeof(struct bag));
    if (bag == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }

    int fd = (op == EPOLLIN) ? conn->rfd : conn->wfd;
    bag->circuit = c;
    bag->conn = conn;
    bag->data = data;
    
    if (ev_arm(fd, op | conn->epollflags, bag)) {
        errorA(c, conn, "_arm");
    }
}


void
writeA(struct circuit *c, struct conn *conn, struct string p) {
    ssize_t size;

    size = write(conn->wfd, p.data, p.size);
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, conn, p, EPOLLIN);
        }
        else {
            errorA(c, conn, "write");
        }
        return;
    }
    RETURN_A(c, conn, NULL);
}


void arrow_io_init(int flags) {
    ev_init(flags);
}


void arrow_io_deinit() {
    ev_deinit();
}
