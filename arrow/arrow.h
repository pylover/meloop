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
    int sint;
    void *ptr;
    struct pair pair;
};


struct element;
struct circuit;


typedef void (*arrow) (struct circuit *c, void* state, union args priv, 
        union args value);
typedef void (*arrow_okcb) (struct circuit *c, void* state, union args value);
typedef void (*arrow_errcb) (struct circuit *c, void* state, const char *msg);


struct circuit * 
newC(arrow_okcb ok, arrow_errcb cb);


void 
freeA(struct circuit *c);


void 
bindA(struct element *e1, struct element *e2);


struct element * 
appendA(struct circuit *c, arrow f, union args priv);


int 
loopA(struct circuit *c1);


void 
returnA(struct circuit *c, void *state, union args result);


void 
errorA(struct circuit *c, void *state, const char *msg);


void
runA(struct circuit *c, void *state, union args);


/* Helper macros */
#define NEW_C(ok, e) newC((arrow_okcb)(ok), (arrow_errcb)(e))
#define APPEND_A(c, a, v) appendA(c, (arrow)(a), (union args)(v))
#define RETURN_A(c, s, r) returnA(c, s, (union args)(r));

#endif
