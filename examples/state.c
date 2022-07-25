#include "arrow/arrow.h"

#include <stdio.h>
#include <stdbool.h>


struct state {
    char error[1024];
};


int 
addA(struct state *s, struct pair p, union args priv) {
    printf("addA, left: %d, right: %d\n", p.left, p.right);
    return p.left + p.right;
}


struct pair 
pairA(struct state *s, union args value, union args priv) {
    struct pair p = {
        .left = value.sint,
        .right = priv.sint
    };
    
    printf("pairA, left: %d, right: %d\n", p.left, p.right);
    return p;
}


int 
divA(struct state *s, struct pair p, union args priv) {
    if (p.right == 0) {
        sprintf(s->error, "Division by zero");
        return 0;
    }
    return p.left / p.right;
}


int
cubeA(struct state *s, int x, union args priv) {
    return x * x * x;
}


void 
main() {
    int out;
    struct state s = {"\0"};
    union args p = {
        .pair = {6, 4}
    };
    
    struct circuit *c = returnC(   (arrow) addA,  (union args) {.null = true});
                        appendC(c, (arrow) pairA, (union args) {.sint = 2});
                        appendC(c, (arrow) divA,  (union args) {.null = true});
                        appendC(c, (arrow) cubeA, (union args) {.null = true});

    out = runA(c, &s, p).sint;
    if (s.error[0]) {
        printf("Error: %s\n", s.error);
    }
    else {
        printf("Out: %d\n", out);
    }
    freeC(c);
}
