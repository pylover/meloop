#include "meloop/arrow.h"
#include "meloop/io.h"
#include "meloop/random.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>


#define BUFFSIZE 16


void 
promptA(struct circuit *c, struct io *io, struct string buff) {
    struct string s = meloop_vars_string_from_ptr(c);
    memcpy(buff.data, s.data, s.size);
    buff.size = s.size;
    RETURN_A(c, io, buff);
}


void
errorcb(struct circuit *c, struct io *io, const char *error) {
    perror(error);
}


void
successcb(struct circuit *c, struct io *io, int out) {
    printf("Out: %d\n", out);
}


int main() {
    meloop_io_init(0);

    char buff[BUFFSIZE] = "\0";
    struct io state = {
        .wfd = STDOUT_FILENO,
        .rfd = STDIN_FILENO,
        .readsize = BUFFSIZE,
    };

    struct circuit *c = NEW_C(successcb, errorcb);

                        APPEND_A(c, random_openA,   NULL      );
    struct element *e = APPEND_A(c, promptA,        "random$ ");
                        APPEND_A(c, readA,          NULL      );
                        APPEND_A(c, writeA,         NULL      );
    loopA(e);

    /* Run circuit */
    runA(c, &state, any_string(string_from_char(buff))); 

    /* Start and wait for event loop */
    if (meloop_io_loop(NULL)) {
        err(1, "meloop_io_loop");
    }
    printf("after loop\n");
    
    meloop_io_deinit();
    freeC(c);
    return EXIT_SUCCESS;
}
