#include "arrow/arrow.h"

#include <stdio.h>
#include <stdbool.h>


struct state {
    char error[1024];
};


void 
addA(struct circuit *c, struct state *s, struct pair p) {
    printf("addA, left: %d, right: %d\n", p.left, p.right);
    returnA(c, s, (union args) (p.left + p.right));
}


void
pairA(struct circuit *c, struct state *s, int value) {
    struct pair p = {
        .left = value,
        .right = c->priv.sint 
    };
    
    printf("pairA, left: %d, right: %d\n", p.left, p.right);
    returnA(c, s, (union args) p);
}


void 
divA(struct circuit *c, struct state *s, struct pair p) {
    if (p.right == 0) {
        sprintf(s->error, "Division by zero");
        returnA(c, s, (union args) NULL);
        return;
    }
    returnA(c, s, (union args) (p.left / p.right));
}


void
cubeA(struct circuit *c, struct state *s, int x) {
    x = x * x * x;
    returnA(c, s, (union args)x);
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
        NEW_A(      addA,  NULL, NULL    );
        APPEND_A(c, pairA, 2,    NULL    );
        APPEND_A(c, divA,  NULL, NULL    );
        APPEND_A(c, cubeA, NULL, callback);

    runA(c, &s, p);
    freeA(c);
}
