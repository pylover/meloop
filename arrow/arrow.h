#ifndef ARROW_H
#define ARROW_H


#include <stdlib.h>
#include <stdbool.h>

#include "arrow/types.h"


#define ERR -1
#define OK 0


struct element;
struct circuit;


typedef void (*arrow) (struct circuit *c, void* state, union any value);
typedef void (*arrow_okcb) (struct circuit *c, void* state, union any value);
typedef void (*arrow_errcb) (struct circuit *c, void* state, const char *msg);


struct circuit * 
newC(arrow_okcb ok, arrow_errcb cb);


void 
freeA(struct circuit *c);


void 
bindA(struct element *e1, struct element *e2);


struct element * 
appendA(struct circuit *c, arrow f, union any vars);


int 
loopA(struct circuit *c1);


void 
returnA(struct circuit *c, void *state, union any result);


void 
errorA(struct circuit *c, void *state, const char *msg);


void
runA(struct circuit *c, void *state, union any);


int
arrow_vars_int(struct circuit *c);


struct string
arrow_vars_string_from_ptr(struct circuit *c);


struct string 
arrow_string(char *s);


/* Helper macros */
#define NEW_C(ok, e) newC((arrow_okcb)(ok), (arrow_errcb)(e))
#define APPEND_A(c, a, v) appendA(c, (arrow)(a), (union any)(v))
#define RETURN_A(c, s, r) returnA(c, s, (union any)(r));

#endif
