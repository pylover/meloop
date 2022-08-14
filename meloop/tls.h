#ifndef MELOOP_TLS_H
#define MELOOP_TLS_H


#include "meloop/tcp.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>


struct tlsclientS {
    struct tcpclientS;
    SSL *ssl;
};


void 
meloop_tls_init();


void 
meloop_tls_deinit();


void 
tlsA(struct circuitS *c, struct ioS *s, union any data);


#endif
