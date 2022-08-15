#ifndef MELOOP_TLS_H
#define MELOOP_TLS_H


#include "meloop/tcp.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>


struct tlsclientS {
    struct tcpclientS;
    SSL *ssl;
    int tlsstatus;
};


void 
meloop_tls_init();


void 
meloop_tls_deinit();


void 
tlsA(struct circuitS *c, struct ioS *s, union any data);


void
tlswriteA(struct circuitS *c, struct ioS *io, struct stringS p);


void
tlsreadA(struct circuitS *c, struct ioS *io, struct stringS p);


#endif
