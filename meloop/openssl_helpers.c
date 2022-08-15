/*
- https://www.openssl.org/docs/man3.0/man7/migration_guide.html
- https://www.feistyduck.com/library/openssl-cookbook/online/
- https://stackoverflow.com/questions/23518079/epoll-based-non-blocking-ssl-read-stuck-in-a-loop
- https://developer.ibm.com/tutorials/l-openssl/
- https://www.openssl.org/docs/man3.0/man3/SSL_get_fd.html
*/
#include "meloop/types.h"
#include "meloop/logging.h"
#include "meloop/openssl_helpers.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#define TLS_VERIFY_DEPTH 5


/* Cipher suites, https://www.openssl.org/docs/manmaster/man1/ciphers.html */
const char* const PREFERRED_CIPHERS = 
    "HIGH:!aNULL:!kRSA:!SRP:!PSK:!CAMELLIA:!RC4:!MD5:!DSS";


openssl_err 
openssl_prepare(SSL_CTX *ctx, SSL **ssl_, int fd, const char * hostname) {
    SSL *ssl;
    int res;
    openssl_err sslerr;

    ssl = *ssl_ = SSL_new(ctx);
    if (ssl == NULL) {
        return ERR_get_error();
    }
    
    /* Set file descriptor */
    /* https://www.openssl.org/docs/manmaster/man3/SSL_set_fd.html */
    SSL_set_fd(ssl, fd);
    
    /* Client mode */
    /* https://www.openssl.org/docs/manmaster/man3/SSL_set_connect_state.html */
    SSL_set_connect_state(ssl);

    // Set hostname */
    /* https://www.openssl.org/docs/crypto/BIO_s_connect.html */
    res = SSL_set1_host(ssl, hostname);
    if (res != 1) {
        return ERR_get_error();
    }

    /* Set cipher list */
    /* https://www.openssl.org/docs/ssl/SSL_CTX_set_cipher_list.html */
    res = SSL_set_cipher_list(ssl, PREFERRED_CIPHERS);
    if (res != 1) {
        return ERR_get_error();
    }

    /* No documentation. See the source code for tls.h and s_client.c */
    res = SSL_set_tlsext_host_name(ssl, hostname);
    if (res != 1) {
        sslerr = ERR_get_error();
        /* Non-fatal, but who knows what cert might be served by an SNI server  */
        /* (We know its the default site's cert in Apache and IIS...)           */
        ERROR("SSL_set_tlsext_host_name: %s", OPENSSL_REASON(sslerr)); 
    }

    /* Enable hostname verification */
    /* https://www.openssl.org/docs/man1.1.1/man3/SSL_set_hostflags.html */
    SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (!SSL_set1_host(ssl, hostname)) {
        return ERR_get_error();
    }
    
    return OK;
}


static int 
openssl_verify_callback(int preverify, X509_STORE_CTX* x509_ctx) {
    /* For error codes, see http://www.openssl.org/docs/apps/verify.html  */
    
    int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
    int err = X509_STORE_CTX_get_error(x509_ctx);
    
    X509* cert = X509_STORE_CTX_get_current_cert(x509_ctx);
    X509_NAME* iname = cert ? X509_get_issuer_name(cert) : NULL;
    X509_NAME* sname = cert ? X509_get_subject_name(cert) : NULL;
    
    INFO("verify_callback (depth=%d)(preverify=%d)", depth, preverify);
   
    if (preverify == 0) {
        if(err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
            ERROR("X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY");
        else if(err == X509_V_ERR_CERT_UNTRUSTED)
            ERROR("X509_V_ERR_CERT_UNTRUSTED");
        else if(err == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN)
            ERROR("X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN");
        else if(err == X509_V_ERR_CERT_NOT_YET_VALID)
            ERROR("X509_V_ERR_CERT_NOT_YET_VALID");
        else if(err == X509_V_ERR_CERT_HAS_EXPIRED)
            ERROR("X509_V_ERR_CERT_HAS_EXPIRED");
        else if(err == X509_V_OK)
            ERROR("X509_V_OK");
        else
            ERROR("%d", err);
    }

    return preverify;
}




openssl_err 
openssl_init(SSL_CTX ** ctx_) {
    int res;
    unsigned long sslerr;
    SSL_CTX *ctx = NULL;

    /* Initialize OpenSSL library internals */
    /* https://www.openssl.org/docs/ssl/SSL_library_init.html */
    SSL_library_init();

    /* Load SSL errors and other related strings into memory */
    /* https://www.openssl.org/docs/crypto/ERR_load_crypto_strings.html */
    SSL_load_error_strings();

    /* Choosing a method for SSL handshake. */
    /* https://www.openssl.org/docs/ssl/SSL_CTX_new.html */
    const SSL_METHOD* method = SSLv23_method();
    if (method == NULL) {
        return ERR_get_error();
    }
    
    /* Create a new SSL Context. */
    /* https://www.openssl.org/docs/man3.0/man3/SSL_CTX_new.html */
    ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        return ERR_get_error();
    }
    *ctx_ = ctx;

    /* Enable verification */
    /* https://www.openssl.org/docs/manmaster/man3/SSL_CTX_set_verify.html */
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, openssl_verify_callback);
    SSL_CTX_set_verify_depth(ctx, TLS_VERIFY_DEPTH);

    /* Remove the most egregious. Because SSLv2 and SSLv3 have been      */
    /* removed, a TLSv1.0 handshake is used. The client accepts TLSv1.0  */
    /* and above. An added benefit of TLS 1.0 and above are TLS          */
    /* extensions like Server Name Indicatior (SNI).                     */
    const long flags = 
        SSL_OP_ALL | 
        SSL_OP_NO_SSLv2 | 
        SSL_OP_NO_SSLv3 | 
        SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(ctx, flags);

    /* http://www.openssl.org/docs/ssl/SSL_CTX_load_verify_locations.html */
    res = SSL_CTX_load_verify_locations(ctx, NULL, "/etc/ssl/certs");
    if (res != 1) {
        sslerr = ERR_get_error();
        /* Non-fatal, but something else will probably break later */
        ERROR("SSL_CTX_load_verify_locations: %s", OPENSSL_REASON(sslerr));
    }

    return OK;
}


/* CIPHERS

 https://www.openssl.org/docs/manmaster/man1/ciphers.html

    "kEECDH:kEDH:kRSA:AESGCM:AES256:AES128:3DES:SHA256:SHA84:SHA1:!aNULL:" \
    "!eNULL:!EXP:!LOW:!MEDIUM!ADH:!AECDH";

TLS 1.2 only
"ECDHE-ECDSA-AES256-GCM-SHA384:"
"ECDHE-RSA-AES256-GCM-SHA384:"
"ECDHE-ECDSA-AES128-GCM-SHA256:"
"ECDHE-RSA-AES128-GCM-SHA256:"

TLS 1.2 only
"DHE-DSS-AES256-GCM-SHA384:"
"DHE-RSA-AES256-GCM-SHA384:"
"DHE-DSS-AES128-GCM-SHA256:"
"DHE-RSA-AES128-GCM-SHA256:"

TLS 1.0 only
"DHE-DSS-AES256-SHA:"
"DHE-RSA-AES256-SHA:"
"DHE-DSS-AES128-SHA:"
"DHE-RSA-AES128-SHA:"

SSL 3.0 and TLS 1.0
"EDH-DSS-DES-CBC3-SHA:"
"EDH-RSA-DES-CBC3-SHA:"
"DH-DSS-DES-CBC3-SHA:"
"DH-RSA-DES-CBC3-SHA";
*/
