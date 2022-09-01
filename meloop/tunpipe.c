#include "meloop/logging.h"
#include "meloop/tunpipe.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

    
struct tuntcpctx {
    struct tunS *tun;
    struct tcpconnS *conn;
};


void
tt_connA(struct circuitS *c, void *state, struct tuntcpctx *ctx) {
    RETURN_A(c, state, ctx->conn);
}


void
tt_tunA(struct circuitS *c, void *state, struct tuntcpctx *ctx) {
    RETURN_A(c, state, ctx->tun);
}


void
tt_ctxA(struct circuitS *c, void *state, struct fileS *file) {
    RETURN_A(c, state, file->ptr);
}


void
read2A(struct circuitS *c, void *s, struct tuntcpctx *ctx, struct ioP *priv) {
    bool any = false;
    ssize_t size;
    struct tunS *tun = ctx->tun;
    struct tcpconnS *conn = ctx->conn;

    /* Read from the tun descriptor */
    size = read(tun->fd, conn->data->blob, priv->readsize);

    /* Check for EOF */
    if (size == 0) {
        ERROR_A(c, s, ctx, "EOF");
        return;
    }

    /* Error | wait */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, ctx, tun->fd, EPOLLIN, priv->epollflags);
        }
        else {
            ERROR_A(c, s, ctx, "tun read");
            return;
        }
    }
    else {
        any = true;
        conn->data->size = size;
    }

    /* Read from the socket descriptor */
    size = read(conn->fd, tun->data->blob, priv->readsize);

    /* Check for EOF */
    if (size == 0) {
        ERROR_A(c, s, ctx, "EOF");
        return;
    }

    /* Error | wait */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, ctx, conn->fd, EPOLLIN, priv->epollflags);
        }
        else {
            ERROR_A(c, s, ctx, "socket read");
            return;
        }
    }
    else {
        any = true;
        tun->data->size = size;
    }

    if (any) {
        RETURN_A(c, s, ctx);
    }
}


void
write2A(struct circuitS *c, void *s, struct tuntcpctx *ctx) {
    DEBUG("write %p", ctx);
    RETURN_A(c, s, ctx);
}


struct elementE *
tunpipeF(struct circuitS *c, struct ioP *io, struct tunP *tun) {
    // TODO: error callbacks

                         APPEND_A(c, tt_tunA,  NULL);
                         APPEND_A(c, tunopenA, tun );
                         APPEND_A(c, tt_ctxA,  NULL);
    struct elementE *e = APPEND_A(c, read2A,   io  );
                         APPEND_A(c, write2A,  io  );
               loopA(e);
    return e;
}


void
run_tunpipeA(struct circuitS *c, void *state, struct tcpconnS *conn, 
        struct tunS *tun, size_t chunksize) {
    struct tuntcpctx *ctx = malloc(sizeof(struct tuntcpctx));

    struct stringS *tundata = malloc(sizeof(struct stringS));
    tundata->blob = malloc(chunksize);
    tundata->size = 0;

    struct stringS *sockdata = malloc(sizeof(struct stringS));
    sockdata->blob = malloc(chunksize);
    sockdata->size = 0;
    
    /* Wiring */
    conn->ptr = ctx;
    conn->data = sockdata;

    tun->ptr = ctx;
    tun->data = tundata;

    ctx->conn = conn;
    ctx->tun = tun;

    /* run */
    RUN_A(c, state, ctx); 
}
