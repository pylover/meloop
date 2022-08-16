#include "meloop/arrow.h"
#include "meloop/types.h"
#include "meloop/io.h"
#include "meloop/logging.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>


#define BUFFSIZE 1024


void 
promptA(struct circuitS *c, struct ioS *io, struct stringS buff) {
    struct stringS s = meloop_priv_string_from_ptr(c);
    memcpy(buff.data, s.data, s.size);
    buff.size = s.size;
    writeA(c, io, buff);
}


void 
echoA(struct circuitS *c, struct ioS *io, struct stringS buff) {
    if ((buff.size == 1) && (buff.data[0] == '\n')) {
        buff.size = 0;
    }
    RETURN_A(c, io, buff);
}


void
errorcb(struct circuitS *c, struct ioS *io, union any, const char *error) {
    ERROR(error);
}


int main() {
    logging_verbosity = LOGGING_DEBUG;
    meloop_io_init(0);

    char buff[BUFFSIZE] = "\0";
    struct pipeS state = {
        .wfd = STDOUT_FILENO,
        .rfd = STDIN_FILENO,
        .readsize = BUFFSIZE,
    };

    struct circuitS *c = NEW_C(NULL, errorcb);

    struct elementS *e = APPEND_A(c, promptA, meloop_string("me@loop:~$ "));
                         APPEND_A(c, readA,   NULL);
                         APPEND_A(c, echoA,   NULL);
                         APPEND_A(c, writeA,  NULL);
    loopA(e);

    /* Run circuitS */
    RUN_A(c, &state, meloop_string(buff)); 

    /* Start and wait for event loop */
    if (meloop_io_loop(NULL)) {
        err(1, "meloop_io_loop");
    }
    
    meloop_io_deinit();
    freeC(c);
    return EXIT_SUCCESS;
}
