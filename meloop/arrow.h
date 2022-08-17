#ifndef MELOOP_H
#define MELOOP_H


#include <stdlib.h>
#include <stdbool.h>


#define ERR -1
#define OK 0


struct elementS;
struct circuitS;


typedef void (*meloop) (struct circuitS *c, void* state, void *data);
typedef void (*meloop_okcb) (struct circuitS *c, void* state, void *data);
typedef void (*meloop_errcb) (struct circuitS *c, void* state, void *data, 
        const char *msg);


struct circuitS * 
newC(meloop_okcb ok, meloop_errcb cb);


void 
freeC(struct circuitS *c);


void 
bindA(struct elementS *e1, struct elementS *e2);


struct elementS * 
appendA(struct circuitS *c, meloop f, void *data);


int 
loopA(struct elementS *e);


void 
returnA(struct circuitS *c, void *state, void *data);


void 
errorA(struct circuitS *c, void *state, void *data, const char *format, 
        ...);


void
runA(struct circuitS *c, void *state, void *data);


void *
meloop_priv_ptr(struct circuitS *c);


/* Helper macros */
#define NEW_C(ok, e) newC((meloop_okcb)(ok), (meloop_errcb)(e))
#define APPEND_A(c, a, v) appendA(c, (meloop)(a), (void*)(v))
#define RETURN_A(c, s, r) returnA(c, s, (void*)(r))
#define ERROR_A(c, s, r, ...) errorA(c, s, (void*)(r), __VA_ARGS__)
#define RUN_A(c, s, d) runA(c, s, (void*)(d))


#endif
