#ifndef ARROW_H
#define ARROW_H


#include <stdlib.h>
#include <stdbool.h>

#include "arrow/types.h"


struct element;
struct circuit;


typedef void (*arrow) (struct circuit *c, void* state, union any value);
typedef void (*arrow_okcb) (struct circuit *c, void* state, union any value);
typedef void (*arrow_errcb) (struct circuit *c, void* state, union any value, 
        const char *msg);


struct circuit * 
newC(arrow_okcb ok, arrow_errcb cb);


void 
freeC(struct circuit *c);


void 
bindA(struct element *e1, struct element *e2);


struct element * 
appendA(struct circuit *c, arrow f, union any vars);


int 
loopA(struct element *e);


void 
returnA(struct circuit *c, void *state, union any result);


void 
errorA(struct circuit *c, void *state, union any data, const char *format, 
        ...);


void
runA(struct circuit *c, void *state, union any);


int
arrow_vars_int(struct circuit *c);


struct string
arrow_vars_string_from_ptr(struct circuit *c);


/* Helper macros */
#define NEW_C(ok, e) newC((arrow_okcb)(ok), (arrow_errcb)(e))
#define APPEND_A(c, a, v) appendA(c, (arrow)(a), (union any)(v))
#define RETURN_A(c, s, r) returnA(c, s, (union any)(r))
#define ERROR_A(c, s, r, ...) errorA(c, s, (union any)(r), __VA_ARGS__)


#endif
