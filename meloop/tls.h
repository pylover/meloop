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
tlsconnectA(struct circuitS *c, struct ioS *s, union any data);


#endif
