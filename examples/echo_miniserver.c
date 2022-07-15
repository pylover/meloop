#include "monad/monad.h"
#include "monad/io.h"
#include "monad/tcp.h"

#include <stdlib.h>


#define CHUNK_SIZE  1024
#define OK 0
static struct io_props clientdev = {false, CHUNK_SIZE};


int main() {
    monad_io_init(0);
    
    struct bind bindinfo = {
        .host = "127.0.0.1",
        .port = 9090,
        .client_monad = echoF(&clientdev),
    };
    
    monad_tcp_runserver(&bindinfo, NULL);
    monad_free(bindinfo.client_monad);
    monad_io_deinit();
    return OK;
}
