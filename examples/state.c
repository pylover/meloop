#include "meloop/arrow.h"

#include <stdio.h>
#include <stdbool.h>


struct state {
    char error[1024];
};


void 
addA(struct circuitS *c, struct state *s, struct pairS p) {
    printf("addA, left: %d, right: %d\n", p.left, p.right);
    RETURN_A(c, s, p.left + p.right);
}


void
pairA(struct circuitS *c, struct state *s, int value) {
    struct pairS p = {
        .left = value,
        .right = meloop_vars_int(c)
    };
    
    printf("pairA, left: %d, right: %d\n", p.left, p.right);
    RETURN_A(c, s, p);
}


void 
divA(struct circuitS *c, struct state *s, struct pairS p) {
    if (p.right == 0) {
        ERROR_A(c, s, p, "Division by zero");
        return;
    }
    RETURN_A(c, s, p.left / p.right);
}


void
cubeA(struct circuitS *c, struct state *s, int x) {
    x = x * x * x;
    RETURN_A(c, s, x);
}


void
errorcb(struct circuitS *c, struct state *s, const char *error) {
    printf("Error: %s\n", error);
}


void
successcb(struct circuitS *c, struct state *s, int out) {
    printf("Out: %d\n", out);
}


void 
main() {
    struct state s = {"\0"};
    union any p = {
        .pair = {6, 4}
    };
    
    struct circuitS *c = NEW_C(successcb, errorcb);
    APPEND_A(c, addA,  NULL);
    APPEND_A(c, pairA, 2   );
    APPEND_A(c, divA,  NULL);
    APPEND_A(c, cubeA, NULL);

    runA(c, &s, p);
    freeC(c);
}
