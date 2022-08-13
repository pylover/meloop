#include "meloop/tls.h"
#include "meloop/logging.h"
#include "meloop/openssl_helpers.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


static SSL_CTX *ctx;


/* Initializing OpenSSL */
void meloop_tls_init() {
    int res;
    unsigned long sslerr;

    /* Initialize openssl library */
    if (openssl_init(&ctx) != OK) {
        FATAL("openssl_init");
    }
}


void meloop_tls_deinit() {
    if (ctx == NULL) {
        return;
    }

    SSL_CTX_free(ctx);
}
