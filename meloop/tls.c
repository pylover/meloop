#include "meloop/tls.h"
#include "meloop/logging.h"
#include "meloop/openssl_helpers.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


static SSL_CTX *ctx;


void 
tlsA(struct circuitS *c, struct ioS *s, union any data) {
    openssl_err sslerr;

    /* Prepare for ssl handshake */
    if (openssl_preconnect(ctx, &(t->ssl), tcp_fd(t->tcp), 
                t->hostname) != OK) {
        FATAL("Openssl: connect error");; 
    }
    
    
    /* https://www.openssl.org/docs/man1.1.1/man3/SSL_connect.html */
    res = SSL_connect(t->ssl);
    if (res != 1) {
        sslerr = SSL_get_error(t->ssl, res);
        
        /* Non-blocking operation did not complete. Try again later. */
        switch (sslerr) {
            case OK:
                INFO("SSL OK");
            case SSL_ERROR_WANT_READ:
                tls_want_read(t, (tcp_callback) _tls_tryconnect, t);
                break;
            case SSL_ERROR_WANT_WRITE:
                tls_want_write(t, (tcp_callback) _tls_tryconnect, t);
                break;
            case SSL_ERROR_WANT_X509_LOOKUP:
                // TODO: use timerfd_create(2) to retry
                FATAL("SSL_ERROR_WANT_X509_LOOKUP");
                break;
            default:
                FATAL("Error SSL_connect: %ld", sslerr);
                break;
        }
    }
    
    /* TODO: verify */
    /* Connection Success, registering fd write */
    tls_want_write(t, t->callback, t->ptr);

}


/* Initializing OpenSSL */
void 
meloop_tls_init() {
    int res;
    unsigned long sslerr;

    /* Initialize openssl library */
    if (openssl_init(&ctx) != OK) {
        FATAL("openssl_init");
    }
}


void 
meloop_tls_deinit() {
    if (ctx == NULL) {
        return;
    }

    SSL_CTX_free(ctx);
}
