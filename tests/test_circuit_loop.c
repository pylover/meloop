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


void
fooA(struct circuitS *c, struct state *s, int *data) {
    s->foo++;
    (*data)++;
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
    struct state state = {0, 0, false, ""};
    int data = 0;
    struct circuitS *c = NEW_C(ok, oops);
    struct elementE *e = APPEND_A(c, fooA, NULL);
               loopA(e); 
    
    RUN_A(c, &state, &data);
    eqint(data, 10);
    eqint(state.ok, false);
    eqint(state.foo, 10);
    eqstr(state.error, "All done");
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
