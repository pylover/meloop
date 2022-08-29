#include "testing.h"
#include "meloop/arrow.h"


#define CHUNK_SIZE  256


struct state {
    bool foo;
    bool bar;
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
test_foo() {
    static char b[CHUNK_SIZE];
    struct state state = {false, false, false, '\0'};
    int data = 0;
    struct circuitS *c = NEW_C(oops);
            APPEND_A(c, fooA, NULL);
    
    RUN_A(c, &state, &data);
    eqint(data, 1);
    eqbool(state.foo, true);
    eqstr(state.error, "");
    freeC(c);
}


void
test_foobar() {
    static char b[CHUNK_SIZE];
    struct state state = {false, false, false, '\0'};
    int data = 0;
    struct circuitS *c = NEW_C(oops);
            APPEND_A(c, fooA, NULL);
            APPEND_A(c, barA, NULL);
    
    RUN_A(c, &state, &data);
    eqint(data, 2);
    eqbool(state.foo, true);
    eqbool(state.bar, true);
    eqstr(state.error, "");
    freeC(c);
}


void main() {
    test_foo();
    test_foobar();
}
