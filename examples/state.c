#include "arrow/arrow.h"

#include <stdio.h>


struct state {
    char error[1024];
};


int 
addA(struct state *s, struct pair p) {
    return p.left + p.right;
}


int 
divA(struct state *s, struct pair p) {
    if (p.right == 0) {
        sprintf(s->error, "Division by zero");
        return 0;
    }
    return p.left / p.right;
}


int
cubeA(struct state *s, int x) {
    return x * x * x;
}


void 
main() {
    int out;
    struct state s = {"\0"};
    union args p = {
        .pair = {3, 4}
    };
    
    struct circuit *c = returnC(   (arrow) addA);
                        appendC(c, (arrow) cubeA);

    // out = addM(&s, p);
    out = runA(c, &s, p).sint;
    if (s.error[0]) {
        printf("Error: %s\n", s.error);
    }
    else {
        printf("Out: %d\n", out);
    }
    freeC(c);
}
