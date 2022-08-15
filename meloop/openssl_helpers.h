#ifndef MELOOP_OPENSSL_HELPERS_H
#define MELOOP_OPENSSL_HELPERS_H


#include <openssl/ssl.h>


typedef unsigned long openssl_err;


// TODO: deinit function
openssl_err 
openssl_init(SSL_CTX ** ctx_);


openssl_err 
openssl_prepare(SSL_CTX *ctx, SSL **ssl_, int fd, const char * hostname);


#define OPENSSL_REASON(e) ERR_reason_error_string(e)


#endif
