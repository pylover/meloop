#ifndef MELOOP_OPENSSL_HELPERS_H
#define MELOOP_OPENSSL_HELPERS_H


#include <openssl/ssl.h>


typedef unsigned long openssl_err;


openssl_err 
openssl_init(SSL_CTX ** ctx_);


void 
openssl_fatal(unsigned long err, const char * const label);


void 
openssl_error(unsigned long err, const char * const label);


openssl_err 
openssl_verify(SSL_CTX *ctx, SSL *ssl);


openssl_err 
openssl_preconnect(SSL_CTX *ctx, SSL **ssl_, int fd,
        const char * hostname);


#endif
