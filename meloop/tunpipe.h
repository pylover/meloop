#ifndef MELOOP_TUNPIPE_H
#define MELOOP_TUNPIPE_H


#include "meloop/tuntap.h"
#include "meloop/arrow.h"
#include "meloop/tcp.h"


struct tuntcpctx;


void
tt_connA(struct circuitS *c, void *state, struct tuntcpctx *ctx);


void
tt_tunA(struct circuitS *c, void *state, struct tuntcpctx *ctx);


void
tt_ctxA(struct circuitS *c, void *state, struct fileS *file);


void
read2A(struct circuitS *c, void *s, struct tuntcpctx *ctx, struct ioP *priv);


void
write2A(struct circuitS *c, void *state, struct tuntcpctx *ctx);


struct elementE *
tunpipeF(struct circuitS *c, struct ioP *io, struct tunP *tun);


void
run_tunpipeA(struct circuitS *c, void *state, struct tcpconnS *conn, 
        struct tunS *tun, size_t chunksize);


#endif
