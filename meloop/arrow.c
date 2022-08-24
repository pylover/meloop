#include "meloop/arrow.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <err.h>


#define MELOOP_ERROR_BUFFSIZE    1024


enum meloop_element_type {
    MLETSIMPLE = 1,
    MLERFORK   = 2
};


struct forkE {
    struct elementE *nextleft;
    struct elementE *nextright;
};


struct simpleE {
    struct elementE *next;
};


struct elementE {
    enum meloop_element_type type;
    meloop run;
    void *priv;
    bool last;
    union {
        struct simpleE simple;
        struct forkE fork;
    };
};


// TODO: reanme to circuitC
struct circuitS {
    meloop_okcb ok;
    meloop_errcb err;
    struct elementE *current;
    struct elementE *nets;
};


struct circuitS * 
newC(meloop_okcb ok, meloop_errcb error) {
    struct circuitS *c = malloc(sizeof(struct circuitS));
    if (c == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }
    
    c->ok = ok;
    c->err = error;
    c->current = NULL;
    c->nets = NULL;
    return c;
}


/** 
  Make elementE e2 from conputation and bind it to c. returns e2.

  c     meloop   result

  o-o   O       o-o-O 

  o-o   O       o-o-O
  |_|           |___|

*/
struct elementE * 
appendA(struct circuitS *c, meloop f, void *priv) {
    struct elementE *e2 = malloc(sizeof(struct elementE));
    if (e2 == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }
    
    /* Initialize new elementE */
    e2->type = MLETSIMPLE;
    e2->run = f;
    e2->priv = priv;
    e2->last = true;
    e2->simple.next = NULL;
    
    if (c->nets == NULL) {
        c->nets = e2;
    }
    else {
        /* Bind them */
        bindA(c->nets, e2);
    }
    return e2;
}


static void 
freeE(struct elementE *e) {
    if (e == NULL) {
        return;
    }
    
    bool last = e->last;
    struct elementE *next = e->simple.next;
    free(e);

    if (last) {
        /* Disposition comppleted. */
        return;
    }

    freeE(next);
}


void 
freeC(struct circuitS *c) {
    if (c == NULL) {
        return;
    }
    
    freeE(c->nets);
    free(c);
}

/** 
  Bind two circuitSs:

  e1    e2     result

  o-o   O-O    o-o-O-O 

  o-o   O-O    o-o-O-O 
        |_|        |_|

  o-o   O-O    o-o-O-O
  |_|          |_____|

  o-o   O-O    o-o-O-O
  |_|   |_|    |_____|

*/
void 
bindA(struct elementE *e1, struct elementE *e2) {
    struct elementE *e1last = e1;
    struct elementE *e2last = e2;

    while (true) {
        /* Open cicuit */
        if (e1last->simple.next == NULL) {
            /* e1 Last elementE */
            e1last->simple.next = e2;
            e1last->last = false;
            return;
        }

        /* Closed cicuit */
        if (e1last->simple.next == e1) {
            /* It's a closed loop, Inserting e2 before the first elementE. */
            e1last->simple.next = e2;
            e1last->last = false;
            while (true) {
                /* e2 Last elementE */
                if ((e2last->simple.next == NULL) || 
                        (e2last->simple.next == e2)) {
                    e2last->simple.next = e1;
                    return;
                }
            
                /* Navigate forward */
                e2last = e2last->simple.next;
            }
            return;
        }
        
        /* Try next circuitS */
        e1last = e1last->simple.next;
    }
}


/** 
  Close (Loop) the circuitS c.

  Syntactic sugar for bindA(e2, e1) if e1 is the first elementE in the 
  circuitS and e2 is the last one.
    
  If the c1 is already a closed circuitS, then 1 will be returned.
  Otherwise the returned value will be zero.

  c     result

  o-o   o-o
        |_|

  o-o   err 
  |_|   

*/
int 
loopA(struct elementE *e) {
    struct elementE *first = e;
    struct elementE *last = e;
    while (true) {
        if (last->simple.next == NULL) {
            /* Last elementE */
            last->simple.next = first;
            last->last = true;
            return OK;
        }

        if (last->simple.next == first) {
            /* It's already a closed loop, Do nothing. */
            return ERR;
        }

        last = last->simple.next;
    }
}


void 
returnA(struct circuitS *c, void *state, void *data) {
    struct elementE *curr = c->current;
    if (curr->simple.next == NULL) {
        if (c->ok != NULL) {
            c->ok(c, state, data);
        }
        c->current = NULL;
        return;
    }

    struct elementE *next = curr->simple.next;
    c->current = next;
    next->run(c, state, data);
}


void 
errorA(struct circuitS *c, void *state, void *data, const char *format, 
        ...) {
    char buff[MELOOP_ERROR_BUFFSIZE]; 
    char *msg;
    va_list args;

    /* var args */
    if (format != NULL) {
        va_start(args, format);
        vsnprintf(buff, MELOOP_ERROR_BUFFSIZE, format, args);
        va_end(args);
        msg = buff;
    }
    else {
        msg = NULL;
    }

    if (c->err != NULL) {
        c->err(c, state, data, msg);
    }
    c->current = NULL;
}


void
runA(struct circuitS *c, void *state, void *data) {
    if (c->current == NULL) {
        c->current = c->nets;
    }
    c->current->run(c, state, data);
}


void *
meloop_priv_ptr(struct circuitS *c) {
    return c->current->priv;
}
