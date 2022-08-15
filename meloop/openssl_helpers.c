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
        sslerr = ERR_get_error();
        openssl_error(sslerr, "SSL_new");
        return sslerr;
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
    // char hostname_and_port[strlen(hostname) + 7];
    // sprintf(hostname_and_port, "%s:%d", hostname, port);
    // res = BIO_set_conn_hostname(bio, hostname_and_port);
    if (res != 1) {
        sslerr = ERR_get_error();
        openssl_error(sslerr, "BIO_set_conn_hostname");
        return sslerr;
    }

    /* Set cipher list */
    /* https://www.openssl.org/docs/ssl/SSL_CTX_set_cipher_list.html */
    res = SSL_set_cipher_list(ssl, PREFERRED_CIPHERS);
    if (res != 1) {
        sslerr = ERR_get_error();
        openssl_error(sslerr, "SSL_set_cipher_list");
        return sslerr;
    }

    /* No documentation. See the source code for tls.h and s_client.c */
    res = SSL_set_tlsext_host_name(ssl, hostname);
    if (res != 1) {
        sslerr = ERR_get_error();
        /* Non-fatal, but who knows what cert might be served by an SNI server  */
        /* (We know its the default site's cert in Apache and IIS...)           */
        openssl_error(sslerr, "SSL_set_tlsext_host_name");
    }

    /* Enable hostname verification */
    /* https://www.openssl.org/docs/man1.1.1/man3/SSL_set_hostflags.html */
    SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (!SSL_set1_host(ssl, hostname)) {
        sslerr = ERR_get_error();
        openssl_error(sslerr, "SSL_set1_host");
        return sslerr;
    }
    
    return OK;
    /* TODO: remove it */
    /* Make it non-blocking */
    // https://stackoverflow.com/questions/39058675/non-blocking-bio-and-hang-after-bio-do-connect
    /* https://www.openssl.org/docs/man1.1.1/man3/BIO_set_nbio.html */
    /* https://developer.ibm.com/tutorials/l-openssl/ */
    /* https://github.com/mdaxini/howto-openssl/blob/master/src/epoll_tls_client.c */
    /* No exception handling required. it returns always 1. */
    //BIO_set_nbio(bio, 1);

    // /* https://www.openssl.org/docs/crypto/BIO_s_connect.html */
    // res = BIO_do_connect(bio);
    // if (res != 1) {
    //     sslerr = ERR_get_error();
    //     openssl_error(sslerr, "BIO_do_connect");
    //     return sslerr;
    // }

    // /* Handshake */
    // /* https://www.openssl.org/docs/crypto/BIO_f_ssl.html */
    // res = BIO_do_handshake(bio);
    // if (res != 1) {
    //     sslerr = ERR_get_error();
    //     openssl_error(sslerr, "BIO_do_handshake");
    //     return sslerr;
    // }
}


/*
You need to perform X509 verification here. There are two documents that 
provide guidance on the gyrations. First is RFC 5280, and second is RFC 6125. 
Two other documents of interest are:                                                       

    Baseline Certificate Requirements:                                             
      https://www.cabforum.org/Baseline_Requirements_V1_1_6.pdf                    
    Extended Validation Certificate Requirements:                                  
      https://www.cabforum.org/Guidelines_v1_4_3.pdf                               
                                                                                   
Here are the minimum steps you should perform:                                     

  1. Call SSL_get_peer_certificate and ensure the certificate is non-NULL. It      
     should never be NULL because Anonymous Diffie-Hellman (ADH) is not 
     allowed.   
  2. Call SSL_get_verify_result and ensure it returns X509_V_OK. This return 
     value depends upon your verify_callback if you provided one. If not, the 
     library default validation is fine (and you should not need to change 
     it).            
  3. Verify either the CN or the SAN matches the host you attempted to connect 
     to. Note Well (N.B.): OpenSSL prior to version 1.1.0 did *NOT* perform 
     hostname verification. If you are using OpenSSL 0.9.8 or 1.0.1, then you 
     will need to perform hostname verification yourself. The code to get you 
     started on hostname verification is provided in print_cn_name and 
     print_san_name. Be sure you are sensitive to ccTLDs (don't navively 
     transform the hostname string). http://publicsuffix.org/ might be 
     helpful.                           
                                                                                   
If all three checks succeed, then you have a chance at a secure connection. 
But its only a chance, and you should either pin your certificates (to remove 
DNS, CA, and Web Hosters from the equation) or implement a Trust-On-First-Use 
(TOFU) scheme like Perspectives or SSH. But before you TOFU, you still have to 
make the customary checks to ensure the certifcate passes the sniff test.             
*/                                                                                   
openssl_err 
openssl_verify(SSL_CTX *ctx, SSL *ssl) {
    int res;
    openssl_err sslerr;

    /* Step 1: verify a server certifcate was presented during negotiation */
    /* https://www.openssl.org/docs/ssl/SSL_get_peer_certificate.html */
    X509* cert = SSL_get_peer_certificate(ssl);
    if (cert) { 
        /* Free immediately */
        X509_free(cert); 
    }
    if (NULL == cert) {
        /* Hack a code for print_error_string. */
        openssl_error(X509_V_ERR_APPLICATION_VERIFICATION, 
                "SSL_get_peer_certificate");
        return X509_V_ERR_APPLICATION_VERIFICATION;
    }
    
    /* Step 2: verify the result of chain verifcation             */
    /* http://www.openssl.org/docs/ssl/SSL_get_verify_result.html */
    /* Error codes: http://www.openssl.org/docs/apps/verify.html  */
    res = SSL_get_verify_result(ssl);
    if (res != X509_V_OK) {
        /* Hack a code into print_error_string. */
        openssl_error((openssl_err)res, "SSL_get_verify_results");
        return res;
    }
    
    /* Step 3: hostname verifcation. */
    /* Hostname verification performs internally by OpenSSL and already 
       enabled within openssl_connect method in this module. */
}


void inline 
openssl_error(unsigned long err, const char * const label) {
    const char* const str = ERR_reason_error_string(err);
    if (str) {
        ERROR("%s", str);
    }
    else {
        ERROR("%s failed: %lu (0x%lx)", label, err, err);
    }
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
        sslerr = ERR_get_error();
        openssl_error(sslerr, "SSLv23_method");
        return sslerr;
    }
    
    /* Create a new SSL Context. */
    /* https://www.openssl.org/docs/man3.0/man3/SSL_CTX_new.html */
    ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        sslerr = ERR_get_error();
        openssl_error(sslerr, "SSL_CTX_new");
        return sslerr;
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
        openssl_error(sslerr, "SSL_CTX_load_verify_locations");
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
