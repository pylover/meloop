#include <stdio.h>


struct pair {
    int left;
    int right;
};


struct state {
    char error[1024];
};


int addM(struct state *s, struct pair p) {
    return p.left + p.right;
}


int divM(struct state *s, struct pair p) {
    if (p.right == 0) {
        sprintf(s->error, "Division by zero");
        return 0;
    }
    return p.left / p.right;
}


void main() {
    struct state s = {"\0"};
    struct pair p = {2, 0};

    //int out = addM(&s, p);
    int out = divM(&s, p);
    if (s.error[0]) {
        printf("Error: %s\n", s.error);
    }
    else {
        printf("Out: %d\n", out);
    }
}
