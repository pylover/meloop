#ifndef MELOOP_H
#define MELOOP_H


#include <stdlib.h>
#include <stdbool.h>

#include "meloop/types.h"


struct elementS;
struct circuitS;


typedef void (*meloop) (struct circuitS *c, void* state, union any value);
typedef void (*meloop_okcb) (struct circuitS *c, void* state, union any value);
typedef void (*meloop_errcb) (struct circuitS *c, void* state, union any value, 
        const char *msg);


struct circuitS * 
newC(meloop_okcb ok, meloop_errcb cb);


void 
freeC(struct circuitS *c);


void 
bindA(struct elementS *e1, struct elementS *e2);


struct elementS * 
appendA(struct circuitS *c, meloop f, union any vars);


int 
loopA(struct elementS *e);


void 
returnA(struct circuitS *c, void *state, union any result);


void 
errorA(struct circuitS *c, void *state, union any data, const char *format, 
        ...);


void
runA(struct circuitS *c, void *state, union any);


int
meloop_vars_int(struct circuitS *c);


struct stringS
meloop_vars_string_from_ptr(struct circuitS *c);


/* Helper macros */
#define NEW_C(ok, e) newC((meloop_okcb)(ok), (meloop_errcb)(e))
#define APPEND_A(c, a, v) appendA(c, (meloop)(a), (union any)(v))
#define RETURN_A(c, s, r) returnA(c, s, (union any)(r))
#define ERROR_A(c, s, r, ...) errorA(c, s, (union any)(r), __VA_ARGS__)
#define RUN_A(c, s, d) runA(c, s, (union any)(d))


#endif
