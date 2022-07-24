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


void 
main() {
    int out;
    struct state s = {"\0"};
    struct pair p = {2, 0};

    // out = addM(&s, p);
    out = divA(&s, p);
    if (s.error[0]) {
        printf("Error: %s\n", s.error);
    }
    else {
        printf("Out: %d\n", out);
    }
}
