#include "testing.h"
#include "meloop/arrow.h"


#define CHUNK_SIZE  256


struct state {
    bool foo;
    bool bar;
    bool ok;
    char error[CHUNK_SIZE];
};


void
fooA(struct circuitS *c, struct state *s, int *data) {
    s->foo = true;
    (*data)++;
    RETURN_A(c, s, data);
}


void
barA(struct circuitS *c, struct state *s, int *data) {
    s->bar = true;
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
test_foo() {
    static char b[CHUNK_SIZE];
    struct state state = {false, false, ""};
    int data = 0;
    struct circuitS *c = NEW_C(ok, oops);
            APPEND_A(c, fooA, NULL);
    
    RUN_A(c, &state, &data);
    eqint(data, 1);
    eqbool(state.ok, true);
    eqbool(state.foo, true);
    eqstr(state.error, "");
}


void
test_foobar() {
    static char b[CHUNK_SIZE];
    struct state state = {false, false, ""};
    int data = 0;
    struct circuitS *c = NEW_C(ok, oops);
            APPEND_A(c, fooA, NULL);
            APPEND_A(c, barA, NULL);
    
    RUN_A(c, &state, &data);
    eqint(data, 2);
    eqbool(state.ok, true);
    eqbool(state.foo, true);
    eqbool(state.bar, true);
    eqstr(state.error, "");
}


void main() {
    test_foo();
    test_foobar();
    // TODO: test private
    // TODO: test chain
    // TODO: test loop
    // TODO: test fork
}
