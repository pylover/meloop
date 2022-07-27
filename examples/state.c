#include "arrow/arrow.h"

#include <stdio.h>
#include <stdbool.h>


struct state {
    char error[1024];
};


void 
addA(struct circuit *c, struct state *s, union args priv, union args args) {
    printf("addA, left: %d, right: %d\n", args.pair.left, args.pair.right);
    struct pair p = args.pair;
    RETURN_A(c, s, p.left + p.right);
}


void
pairA(struct circuit *c, struct state *s, union args priv, union args value) {
    struct pair p = {
        .left = value.sint,
        .right = priv.sint
    };
    
    printf("pairA, left: %d, right: %d\n", p.left, p.right);
    RETURN_A(c, s, p);
}


void 
divA(struct circuit *c, struct state *s, union args priv, struct pair p) {
    if (p.right == 0) {
        errorA(c, s, "Division by zero");
        return;
    }
    RETURN_A(c, s, p.left / p.right);
}


void
cubeA(struct circuit *c, struct state *s, union args priv, int x) {
    x = x * x * x;
    RETURN_A(c, s, x);
}


void
errorcb(struct circuit *c, struct state *s, const char *error) {
    printf("Error: %s\n", error);
}


void
successcb(struct circuit *c, struct state *s, int out) {
    printf("Out: %d\n", out);
}


void 
main() {
    struct state s = {"\0"};
    union args p = {
        .pair = {6, 4}
    };
    
    struct circuit *c = NEW_C(successcb, errorcb);
    APPEND_A(c, addA,  NULL);
    APPEND_A(c, pairA, 2   );
    APPEND_A(c, divA,  NULL);
    APPEND_A(c, cubeA, NULL);

    runA(c, &s, p);
    freeA(c);
}
