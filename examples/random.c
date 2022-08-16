#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/random.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>


#define BUFFSIZE 32


void 
promptA(struct circuitS *c, struct fileS *io, struct stringS buff) {
    struct stringS s = meloop_priv_string_from_ptr(c);
    memcpy(buff.data, s.data, s.size);
    buff.size = s.size;
    writeA(c, io, buff);
}


void
errorcb(struct circuitS *c, struct fileS *io, struct stringS d, const char *e) {
    perror(e);
}


void
successcb(struct circuitS *c, struct fileS *io, int out) {
    printf("Out: %d\n", out);
}


int main() {
    meloop_io_init(0);

    char buff[BUFFSIZE] = "\0";
    struct randS rand = {
        .fd = -1,
    };

    struct fileS state = {
        .wfd = STDOUT_FILENO,
        .rfd = STDIN_FILENO,
        .readsize = BUFFSIZE,
    };

    struct circuitS *c = NEW_C(successcb, errorcb);

                         APPEND_A(c, randopenA, meloop_ptr(&rand));
    struct elementS *e = APPEND_A(c, readA,     NULL);
                         APPEND_A(c, randreadA, meloop_ptr(&rand));
                         APPEND_A(c, randencA,  meloop_ptr(&rand));
                         APPEND_A(c, writeA,    NULL);
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
