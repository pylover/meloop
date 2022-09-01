#include "meloop/tunpipe.h"
#include "meloop/arrow.h"

    
struct ctx {
    struct tunS tun;
    struct tcpconnS conn;
};


struct pairC {
    struct circuitS *left;
    struct circuitS *right;
};


void
connA(struct circuitS *c, void *state, struct ctx *ctx) {
    RETURN_A(c, state, ctx->conn);
}


void
tunA(struct circuitS *c, void *state, struct ctx *ctx) {
    RETURN_A(c, state, ctx->tun);
}


void
ctxA(struct circuitS *c, void *state, struct fileS *file) {
    RETURN_A(c, state, file->ptr);
}


void
forkA(struct circuitS *c, void *state, struct ctx *ctx) {
    

    RUN_A(iot, NULL, &pipet); 
    RUN_A(ios, NULL, &pipes); 
    RETURN_A(c, s, d);
}


struct elementE *
tunpipeF(struct ioP *io, struct tunP *tun, meloop_errcb ecb) {
    // TODO: error callbacks
    struct pairC *fork = malloc(sizeof(struct pair));

    /* Tunnel to socket circuit */
    struct circuitS *tun2sock = NEW_C(NULL);
    struct elementE *t2s = APPEND_A(tun2sock, pipereadA,  &io);
                           APPEND_A(tun2sock, pipewriteA, &io);
    loopA(t2s);
    fork->left = tun2sock;

    /* Socket to tunnel circuit */
    struct circuitS *sock2tun = NEW_C(NULL);
    struct elementE *s2t = APPEND_A(sock2tun, pipereadA,  &io);
                           APPEND_A(sock2tun, pipewriteA, &io);
    loopA(s2t);
    fork->right = sock2tun;

    struct circuitS *init = NEW_C(ecb);
    struct elementE *e = APPEND_A(init, tunA,     NULL);
                         APPEND_A(init, tunopenA, tun );
                         APPEND_A(init, ctxA,     NULL);
                         APPEND_A(init, forkA,    NULL);
    
    init->ptr = fork;
    return e;
}


void
run_tunpipeA(struct tcpconnS *conn, struct tunS *tun, size_t chunksize) {
    struct stringS *data = malloc(sizeof(struct stringS));
    data->blob = malloc(chunksize);
    data->size = 0;
    struct ctx *ctx = malloc(sizeof(struct ctx));
    
    /* Wiring */
    conn->ptr = ctx;
    conn->data = data;

    tun->ptr = ctx;
    tun->data = data;

    ctx->conn = conn;
    ctx->tun = tun;

    /* run */
    RUN_A(init, NULL, ctx); 
}
