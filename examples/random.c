#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/random.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>


#define BUFFSIZE 32


void 
promptA(struct circuit *c, struct io *io, struct string buff) {
    struct string s = meloop_vars_string_from_ptr(c);
    memcpy(buff.data, s.data, s.size);
    buff.size = s.size;
    writeA(c, io, buff);
}


void
encodeA(struct circuit *c, struct io *io, struct string buff) {
    int i;
    unsigned int t;
    for (i = 0; i < buff.size; i++) {
        t = buff.data[i];
        buff.data[i] = (t % 26) + 97;
    }
    RETURN_A(c, io, buff);
}


void
errorcb(struct circuit *c, struct io *io, struct string d, const char *e) {
    perror(e);
}


void
successcb(struct circuit *c, struct io *io, int out) {
    printf("Out: %d\n", out);
}


int main() {
    meloop_io_init(0);

    char buff[BUFFSIZE] = "\0";
    struct rand state = {
        .wfd = STDOUT_FILENO,
        .rfd = STDIN_FILENO,
        .readsize = BUFFSIZE,
        .randfd = -1,
    };

    struct circuit *c = NEW_C(successcb, errorcb);

                        APPEND_A(c, randopenA, NULL);
    struct element *e = APPEND_A(c, readA,     NULL);
                        APPEND_A(c, randreadA, NULL);
                        APPEND_A(c, encodeA,   NULL);
                        APPEND_A(c, writeA,    NULL);
              loopA(e);

    /* Run circuit */
    RUN_A(c, &state, meloop_atos(buff)); 

    /* Start and wait for event loop */
    if (meloop_io_loop(NULL)) {
        err(1, "meloop_io_loop");
    }
    
    meloop_io_deinit();
    freeC(c);
    return EXIT_SUCCESS;
}
