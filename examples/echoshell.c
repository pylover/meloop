#include "arrow/arrow.h"
#include "arrow/io.h"


void 
promptA(struct circuit *c, struct conn *conn) {
    writeA(c, conn, (const char*)c->priv.ptr);
}


int main() {
    arrow_io_init(0);

    struct conn state = {
        .wfd = STDOUT_FILENO,
        .rfd = STDIN_FILENO
    };

    struct circuit *c = NEW_C(successcb, errorcb);
    APPEND_A(c, promptA, ">>");
    
    arrow_io_deinit();
}
