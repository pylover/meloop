#ifndef ARROW_H
#define ARROW_H


#include <stdbool.h>


#define ERR -1
#define OK 0


struct pair {
    int left;
    int right;
};


union args {
    bool null;
    int sint;
    void *ptr;
    struct pair pair;
};


struct circuit;


typedef void (*arrow) (struct circuit *c, void* state, union args value);


struct circuit {
    arrow run;
    arrow callback;    
    union args priv;
    bool last;
    struct circuit *next;
};


struct circuit * 
newA(arrow f, union args priv, arrow cb);


void 
freeA(struct circuit *c);


void 
bindA(struct circuit *c1, struct circuit *c2);


struct circuit * 
appendA(struct circuit *c1, arrow f, union args priv, arrow cb);


int 
loopA(struct circuit *c1);


void 
returnA(struct circuit *c, void *state, union args result);


void
runA(struct circuit *c, void *state, union args);


#endif
