#include "testing.h"
#include "meloop/arrow.h"
#include "meloop/logging.h"


#define CHUNK_SIZE  256


struct state {
    int foo;
    int bar;
    bool ok;
    char error[CHUNK_SIZE];
};


struct fooP {
    int qux;
};


void
fooA(struct circuitS *c, struct state *s, int *data, struct fooP *priv) {
    s->foo++;
    (*data)++;
    priv->qux++;
    if ((*data) >= 10) {
        ERROR_A(c, s, data, "All done");
        return;
    }
    RETURN_A(c, s, data);
}


void
barA(struct circuitS *c, struct state *s, int *data) {
    s->bar++;
    (*data)++;
    RETURN_A(c, s, data);
}


void
oops(struct circuitS *c, struct state *s, int *data, const char *error) {
    strcpy(s->error, error);
}


void
ok(struct circuitS *c, struct state *s, void *data) {
    s->ok = true;
}


void
test_foo_loop() {
    static char b[CHUNK_SIZE];
    int data = 0;
    struct fooP priv = {0};
    struct state state = {0, 0, false, ""};
    struct circuitS *c = NEW_C(ok, oops);
    struct elementE *e = APPEND_A(c, fooA, &priv);
    
    RUN_A(c, &state, &data);
    eqint(data, 1);
    eqint(priv.qux, 1);
    eqint(state.ok, true);
    eqint(state.foo, 1);
    eqstr(state.error, "");
    freeC(c);
}


void
test_foobar_loop() {
    static char b[CHUNK_SIZE];
    struct state state = {0, 0, false, ""};
    int data = 0;
    struct circuitS *c = NEW_C(ok, oops);
    struct elementE *e = APPEND_A(c, fooA, NULL);
                         APPEND_A(c, barA, NULL);
               loopA(e); 
    
    RUN_A(c, &state, &data);
    eqint(11, data);
    eqint(state.ok, false);
    eqint(6, state.foo);
    eqint(5, state.bar);
    eqstr(state.error, "All done");
    freeC(c);
}


void main() {
    test_foo_loop();
    test_foobar_loop();
    // TODO: test private
    // TODO: test fork
}
