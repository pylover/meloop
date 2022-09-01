#include "meloop/tunpipe.h"

    
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
read2A(struct circuitS *c, void *state, struct tuntcpctx *ctx) {

    RETURN_A(c, state, ctx);
}


void
write2A(struct circuitS *c, void *state, struct tuntcpctx *ctx) {

    RETURN_A(c, state, ctx);
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
