#include "io.h"
#include "tcp.h"
#include "monads.h"

#include <stdlib.h>
#include <err.h>


#define TCP_BACKLOG_DEFAULT 2


void monad_tcp_client_free(struct conn *c) {
    if (c->data != NULL) {
        free(c->data);
    }
}


void monad_tcp_runserver(struct bind *info, monad_tcp_finish finish) {
    struct conn listenc = {
        .ptr = info
    };
    struct device dev = {false, 0};
   
    /* Default backlog if not given. */
    if (info->backlog == 0) {
        info->backlog = TCP_BACKLOG_DEFAULT;
    }

    /* Create an open monad chain for listen */
    Monad *listen_m = MONAD_RETURN(          listenM,   &dev);

    /* Create a closed monad chain for accept a connection and wait for the
       next clinet. */
    Monad *accept_m = MONAD_RETURN(          awaitrM, &dev);
                      MONAD_APPEND(accept_m, acceptM, &dev);
    monad_loop(accept_m);
    
    /* Bind them together */
    monad_bind(listen_m, accept_m);

    /* Run it within IO context. */
    if (MONAD_IO_RUN(listen_m, &listenc, (monad_finish)finish)) {
        err(1, "monad_io_run");
    }
   
    /* Dispose */
    monad_free(listen_m);
}
