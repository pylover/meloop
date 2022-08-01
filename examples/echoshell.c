#include "arrow/arrow.h"
#include "arrow/io.h"

#include <err.h>
#include <stdio.h>
#include <unistd.h>


void 
promptA(struct circuit *c, struct conn *conn, struct string buff) {
    struct string s = arrow_vars_string_from_ptr(c);
    memcpy(buff.data, s.data, s.size);
    buff.size = s.size;
    writeA(c, conn, buff);
}


void
errorcb(struct circuit *c, struct conn *conn, const char *error) {
    err(EXIT_FAILURE, "%s", error);
}


void
successcb(struct circuit *c, struct conn *conn, int out) {
    // printf("Out: %d\n", out);
}


#define BUFFSIZE 1024


int main() {
    arrow_io_init(0);
    char *buff = malloc(BUFFSIZE);
    struct conn state = {
        .wfd = STDOUT_FILENO,
        .rfd = STDIN_FILENO,
        .readsize = BUFFSIZE,
    };


    struct circuit *c = NEW_C(successcb, errorcb);
    APPEND_A(c, promptA, string_from_char(">> "));
    APPEND_A(c, readA, any_null());
    APPEND_A(c, writeA, any_null());
    if (loopA(c)) {
        err(1, "loopA");
    }

    /* Run circuit */
    runA(c, &state, any_string(string_from_char(buff))); 

    /* Start and wait for event loop */
    if (arrow_io_loop(NULL)) {
        err(1, "arrow_io_loop");
    }
    arrow_io_deinit();
    free(buff);
}
