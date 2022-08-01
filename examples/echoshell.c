#include "arrow/arrow.h"
#include "arrow/io.h"

#include <stdio.h>
#include <unistd.h>


void 
promptA(struct circuit *c, struct conn *conn) {
    struct string s = arrow_vars_string_from_ptr(c);
    writeA(c, conn, s);
}


void
errorcb(struct circuit *c, struct conn *conn, const char *error) {
    printf("Error: %s\n", error);
}


void
successcb(struct circuit *c, struct conn *conn, int out) {
    // printf("Out: %d\n", out);
}


int main() {
    arrow_io_init(0);
    struct conn state = {
        .wfd = STDOUT_FILENO,
        .rfd = STDIN_FILENO
    };

    struct circuit *c = NEW_C(successcb, errorcb);
    APPEND_A(c, promptA, string_from_char(">>"));
    runA(c, &state, any_null()); 

    // /* Start and wait for event loop */
    // if (arrow_io_loop()) {
    //     err(1, "monad_io_run");
    // }
    arrow_io_deinit();
}
