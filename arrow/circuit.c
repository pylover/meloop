#include "arrow/common.h"
#include "arrow/circuit.h"

#include <stdlib.h>
#include <stdbool.h>
#include <err.h>


struct circuit {
    arrow run;
    union args priv;
    bool last;
    struct circuit *next;
};


struct circuit * 
returnC(arrow f, union args priv) {
    struct circuit *c = malloc(sizeof(struct circuit));
    if (c == NULL) {
        err(EXIT_FAILURE, "Out of memory");
    }
    
    c->run = f;
    c->priv = priv;
    c->last = true;
    c->next = NULL;
    return c;
}


void 
freeC(struct circuit *c) {
    if (c == NULL) {
        return;
    }
    
    bool last = c->last;
    struct circuit *next = c->next;
    free(c);

    if (last) {
        /* Disposition comppleted. */
        return;
    }

    freeC(next);
}


/** 
  Bind two circuits:

  c1    c2     result

  o-o   O-O    o-o-O-O 

  o-o   O-O    o-o-O-O 
        |_|        |_|

  o-o   O-O    o-o-O-O
  |_|          |_____|

  o-o   O-O    o-o-O-O
  |_|   |_|    |_____|

*/
void 
bindA(struct circuit *c1, struct circuit *c2) {
    struct circuit *m1last = c1;
    struct circuit *m2last = c2;

    while (true) {
        /* Open cicuit */
        if (m1last->next == NULL) {
            /* c1 Last element */
            m1last->next = c2;
            m1last->last = false;
            return;
        }

        /* Closed cicuit */
        if (m1last->next == c1) {
            /* It's a closed loop, Inserting c2 before the first element. */
            m1last->next = c2;
            m1last->last = false;
            while (true) {
                /* c2 Last element */
                if ((m2last->next == NULL) || (m2last->next == c2)) {
                    m2last->next = c1;
                    return;
                }
            
                /* Navigate forward */
                m2last = m2last->next;
            }
            return;
        }
        
        /* Try next circuit */
        m1last = m1last->next;
    }
}


/** 
  Make a circuit from conputation and bind it to c1.

  c1    arrow   result

  o-o   O       o-o-O 

  o-o   O       o-o-O
  |_|           |___|

*/
struct circuit * 
appendC(struct circuit *c1, arrow f, union args priv) {
    struct circuit *next = returnC(f, priv);
    bindA(c1, next);
    return next;
}


/** 
  Close (Loop) the circuit c1.

  Syntactic sugar for bindA(c2, c1) if c1 is the first circuit in the 
  chain and c2 is the last one.
    
  If the c1 is already a closed circuit, then 1 will be returned.
  Otherwise the returned value will be zero.

  c1    result

  o-o   o-o
        |_|

  o-o   err 
  |_|   

*/
int 
loopC(struct circuit *c1) {
    struct circuit *last = c1;
    while (true) {
        if (last->next == NULL) {
            /* Last element */
            last->next = c1;
            last->last = true;
            return OK;
        }

        if (last->next == c1) {
            /* It's already a closed loop, Do nothing. */
            return ERR;
        }

        last = last->next;
    }
}


union args runA(struct circuit *c, void *state, union args args) {
    union args result = c->run(state, args, c->priv);
    
    if (c->last || (c->next == NULL)) {
        return result;
    }

    return runA(c->next, state, result);
}
