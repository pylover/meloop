#include "monad/tcp.h"

#include <stdlib.h>


void main() {
    monad_io_init(0);
    static struct io_props client_props = {false, 1024};
    
    struct bind bindinfo = {
        .host = "127.0.0.1",
        .port = 9090,
        .client_monad = echoF(&client_props),
    };
    
    monad_tcp_runserver(&bindinfo, NULL, NULL);
    monad_free(bindinfo.client_monad);
    monad_io_deinit();
}
