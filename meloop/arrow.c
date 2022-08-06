#include "meloop/arrow.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <err.h>


#define MELOOP_ERROR_BUFFSIZE    1024


struct element {
    meloop run;
    union any vars;
    bool last;
    struct element *next;
};


struct circuit {
    meloop_okcb ok;
    meloop_errcb err;
    struct element *current;
    struct element *nets;
};


struct circuit * 
newC(meloop_okcb ok, meloop_errcb error) {
    struct circuit *c = malloc(sizeof(struct circuit));
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
  Make element e2 from conputation and bind it to c. returns e2.

  c     meloop   result

  o-o   O       o-o-O 

  o-o   O       o-o-O
  |_|           |___|

*/
struct element * 
appendA(struct circuit *c, meloop f, union any vars) {
    struct element *e2 = malloc(sizeof(struct element));
    if (e2 == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }
    
    /* Initialize new element */
    e2->run = f;
    e2->vars = vars;
    e2->last = true;
    e2->next = NULL;
    
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
freeE(struct element *e) {
    if (e == NULL) {
        return;
    }
    
    bool last = e->last;
    struct element *next = e->next;
    free(e);

    if (last) {
        /* Disposition comppleted. */
        return;
    }

    freeE(next);
}


void 
freeC(struct circuit *c) {
    if (c == NULL) {
        return;
    }
    
    freeE(c->nets);
    free(c);
}

/** 
  Bind two circuits:

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
bindA(struct element *e1, struct element *e2) {
    struct element *e1last = e1;
    struct element *e2last = e2;

    while (true) {
        /* Open cicuit */
        if (e1last->next == NULL) {
            /* e1 Last element */
            e1last->next = e2;
            e1last->last = false;
            return;
        }

        /* Closed cicuit */
        if (e1last->next == e1) {
            /* It's a closed loop, Inserting e2 before the first element. */
            e1last->next = e2;
            e1last->last = false;
            while (true) {
                /* e2 Last element */
                if ((e2last->next == NULL) || (e2last->next == e2)) {
                    e2last->next = e1;
                    return;
                }
            
                /* Navigate forward */
                e2last = e2last->next;
            }
            return;
        }
        
        /* Try next circuit */
        e1last = e1last->next;
    }
}


/** 
  Close (Loop) the circuit c.

  Syntactic sugar for bindA(e2, e1) if e1 is the first element in the 
  circuit and e2 is the last one.
    
  If the c1 is already a closed circuit, then 1 will be returned.
  Otherwise the returned value will be zero.

  c     result

  o-o   o-o
        |_|

  o-o   err 
  |_|   

*/
int 
loopA(struct element *e) {
    struct element *first = e;
    struct element *last = e;
    while (true) {
        if (last->next == NULL) {
            /* Last element */
            last->next = first;
            last->last = true;
            return OK;
        }

        if (last->next == first) {
            /* It's already a closed loop, Do nothing. */
            return ERR;
        }

        last = last->next;
    }
}


void 
returnA(struct circuit *c, void *state, union any result) {
    struct element *curr = c->current;
    if (curr->next == NULL) {
        if (c->ok != NULL) {
            c->ok(c, state, result);
        }
        c->current = NULL;
        return;
    }

    struct element *next = curr->next;
    c->current = next;
    next->run(c, state, result);
}


void 
errorA(struct circuit *c, void *state, union any data, 
        const char *format, ...) {
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
runA(struct circuit *c, void *state, union any args) {
    struct pair *p = (struct pair*) (&args);
    
    if (c->current == NULL) {
        c->current = c->nets;
    }
    c->current->run(c, state, args);
}


int
meloop_vars_int(struct circuit *c) {
    return c->current->vars.sint;
}


struct string
meloop_vars_string_from_ptr(struct circuit *c) {
    return c->current->vars.string;
}