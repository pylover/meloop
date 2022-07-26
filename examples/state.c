#include "arrow/arrow.h"

#include <stdio.h>
#include <stdbool.h>


struct state {
    char error[1024];
};


void 
addA(struct circuit *c, struct state *s, struct pair p) {
    printf("addA, left: %d, right: %d\n", p.left, p.right);
    returnC(c, s, (union args) (p.left + p.right));
}


void
pairA(struct circuit *c, struct state *s, int value) {
    struct pair p = {
        .left = value,
        .right = c->priv.sint 
    };
    
    printf("pairA, left: %d, right: %d\n", p.left, p.right);
    returnC(c, s, (union args) p);
}


void 
divA(struct circuit *c, struct state *s, struct pair p) {
    if (p.right == 0) {
        sprintf(s->error, "Division by zero");
        returnC(c, s, (union args) NULL);
    }
    returnC(c, s, (union args) (p.left / p.right));
}


void
cubeA(struct circuit *c, struct state *s, int x) {
    x = x * x * x;
    returnC(c, s, (union args)x);
}


void
callback(struct circuit *c, struct state *s, int out) {
    if (s->error[0]) {
        printf("Error: %s\n", s->error);
    }
    else {
        printf("Out: %d\n", out);
    }
}


void 
main() {
    struct state s = {"\0"};
    union args p = {
        .pair = {6, 4}
    };
    
    struct circuit *c = 
        newC(      (arrow) addA,  (union args) NULL, NULL            );
        appendC(c, (arrow) pairA, (union args) 2,    NULL            );
        appendC(c, (arrow) divA,  (union args) NULL, NULL            );
        appendC(c, (arrow) cubeA, (union args) NULL, (arrow) callback);

    runA(c, &s, p);
    freeC(c);
}
