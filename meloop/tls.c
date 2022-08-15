#include "meloop/tls.h"
#include "meloop/logging.h"
#include "meloop/openssl_helpers.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/epoll.h>


static SSL_CTX *ctx;


#define _NEW        1
#define _CONNECTING 2
#define _CONNECTED  3


void
tlsreadA(struct circuitS *c, struct ioS *io, struct stringS p) {
    struct tlsclientS *priv = meloop_priv_ptr(c);
    int size;
    unsigned long sslerr;
    
    size = SSL_read(priv->ssl, p.data, io->readsize);
    // DEBUG("SSL read: %d bytes", size);
    if (size <= 0) {
        sslerr = ERR_get_error();
        if (sslerr = SSL_ERROR_WANT_READ) {
            // DEBUG("ssl want read: %d", sslerr);
            WAIT_A(c, io, p, io->rfd, EPOLLIN);
        }
        else {
            ERROR_A(c, io, p, OPENSSL_REASON(ERR_get_error()));
        }
        return;
    }
    p.size = size;
    RETURN_A(c, io, p);
}


void
tlswriteA(struct circuitS *c, struct ioS *io, struct stringS p) {
    struct tlsclientS *priv = meloop_priv_ptr(c);
    int size;
    unsigned long sslerr;
    
    if (p.size == 0) {
        ERROR_A(c, io, p, "Nothing to write");
        return;
    }
    
    size = SSL_write(priv->ssl, p.data, p.size);
    // DEBUG("SSL write: %d bytes", size);
    if (size <= 0) {
        sslerr = ERR_get_error();
        if (sslerr = SSL_ERROR_WANT_WRITE) {
            // DEBUG("ssl want write: %d", sslerr);
            WAIT_A(c, io, p, io->wfd, EPOLLOUT);
        }
        else {
            ERROR_A(c, io, p, OPENSSL_REASON(ERR_get_error()));
        }
        return;
    }
    RETURN_A(c, io, p);
}


void 
tlsA(struct circuitS *c, struct ioS *s, union any data) {
    struct tlsclientS *priv = meloop_priv_ptr(c);
    int res;
    openssl_err sslerr;
    
    if (priv->tlsstatus != _CONNECTING) {
        /* Prepare for ssl handshake */
        sslerr = openssl_prepare(ctx, &(priv->ssl), s->rfd, priv->hostname);
        if (sslerr != OK) {
            ERROR_A(c, s, data, "openssl_prepare -- %s", 
                    OPENSSL_REASON(sslerr)); 
            return;
        }
        priv->tlsstatus = _CONNECTING;
    }
    
    /* https://www.openssl.org/docs/man1.1.1/man3/SSL_connect.html */
    res = SSL_connect(priv->ssl);
    if (res != 1) {
        sslerr = SSL_get_error(priv->ssl, res);
        
        /* Non-blocking operation did not complete. Try again later. */
        switch (sslerr) {
            case OK:
                INFO("SSL OK");
                break;
            case SSL_ERROR_WANT_READ:
                WAIT_A(c, s, data, s->rfd, EPOLLIN);
                return;
            case SSL_ERROR_WANT_WRITE:
                WAIT_A(c, s, data, s->wfd, EPOLLOUT);
                return;
            case SSL_ERROR_WANT_X509_LOOKUP:
                // TODO: use timerfd_create(2) to retry
                ERROR_A(c, s, data, "SSL_ERROR_WANT_X509_LOOKUP");
                return;
            default:
                ERROR_A(c, s, data, "Error SSL_connect: %ld", sslerr);
                return;
        }
    }
    INFO("SSL Connected");
    priv->tlsstatus = _CONNECTED;
    errno = 0;
    RETURN_A(c, s, data);
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
    openssl_deinit(ctx);
}
