#ifndef MELOOP_TLS_H
#define MELOOP_TLS_H


#include "meloop/tcp.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>


struct tlsclientP {
    struct tcpclientP;
    SSL *ssl;
    int tlsstatus;
};


void 
meloop_tls_init();


void 
meloop_tls_deinit();


void 
tlsA(struct circuitS *c, void *s, struct fileS *f, struct tlsclientP *priv);


void
tlswriteA(struct circuitS *c, void *s, struct fileS *f, 
        struct tlsclientP *priv);


void
tlsreadA(struct circuitS *c, void *s, struct fileS *f, 
        struct tlsclientP *priv);


#endif
