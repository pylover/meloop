#ifndef ARROW_CIRCUIT_H
#define ARROW_CIRCUIT_H


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
newC(arrow f, union args priv, arrow cb);


void 
freeC(struct circuit *c);


void 
bindA(struct circuit *c1, struct circuit *c2);


struct circuit * 
appendC(struct circuit *c1, arrow f, union args priv, arrow cb);


int 
loopC(struct circuit *c1);


void 
returnC(struct circuit *c, void *state, union args result);


void
runA(struct circuit *c, void *state, union args);


#endif
