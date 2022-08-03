#include "arrow/arrow.h"
#include "arrow/io.h"

#include <err.h>
#include <stdio.h>
#include <unistd.h>


#define BUFFSIZE 1024


void 
promptA(struct circuit *c, struct conn *conn, struct string buff) {
    struct string s = arrow_vars_string_from_ptr(c);
    memcpy(buff.data, s.data, s.size);
    buff.size = s.size;
    writeA(c, conn, buff);
}


void 
echoA(struct circuit *c, struct conn *conn, struct string buff) {
    if ((buff.size == 1) && (buff.data[0] == '\n')) {
        buff.size = 0;
    }
    RETURN_A(c, conn, buff);
}


void
errorcb(struct circuit *c, struct conn *conn, const char *error) {
    dprintf(STDERR_FILENO, "%s\n", error);
}


void
successcb(struct circuit *c, struct conn *conn, int out) {
    printf("Out: %d\n", out);
}


int main() {
    arrow_io_init(0);

    char buff[BUFFSIZE] = "\0";
    struct conn state = {
        .wfd = STDOUT_FILENO,
        .rfd = STDIN_FILENO,
        .readsize = BUFFSIZE,
    };

    struct circuit *c = NEW_C(successcb, errorcb);

    struct element *e = APPEND_A(c, promptA, string_from_char("me@loop:~$ "));
                        APPEND_A(c, readA,   NULL);
                        APPEND_A(c, echoA,   NULL);
                        APPEND_A(c, writeA,  NULL);
    loopA(e);

    /* Run circuit */
    runA(c, &state, any_string(string_from_char(buff))); 

    /* Start and wait for event loop */
    if (arrow_io_loop(NULL)) {
        err(1, "arrow_io_loop");
    }
    printf("after loop\n");
    
    arrow_io_deinit();
    freeC(c);
}
