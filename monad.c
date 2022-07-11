#include "monad.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define ERR -1
#define OK 0


struct monad_context {
    struct monad *monad;
    monad_success success;
    monad_failure fail;
};


struct monad {
    monad_task run;
    void *args;
    struct monad *next;
};


// TODO: Use macro instead of function.
static void _run(struct monad_context *ctx, struct monad *m, void *data) {
    m->run(ctx, m->args, data);
}


void monad_run(struct monad *m, void *data, monad_success success, 
        monad_failure fail) {
    struct monad_context *ctx = malloc(sizeof(struct monad_context));
    ctx->monad = m;
    ctx->success = success;
    ctx->fail = fail;
    _run(ctx, m, data);
}


/** 
  Bind two monads:

  m1    m2     result

  o-o   O-O    o-o-O-O 

  o-o   O-O    o-o-O-O 
        |_|        |_|

  o-o   O-O    o-o-O-O
  |_|          |_____|

  o-o   O-O    o-o-O-O
  |_|   |_|    |_____|

*/
void monad_bind(struct monad *m1, struct monad *m2) {
    struct monad *m1last = m1;
    struct monad *m2last = m2;

    while (true) {
        /* Open cicuit */
        if (m1last->next == NULL) {
            /* m1 Last element */
            m1last->next = m2;
            return;
        }

        /* Closed cicuit */
        if (m1last->next == m1) {
            /* It's a closed loop, Inserting m2 before the first element. */
            m1last->next = m2;
            while (true) {
                /* m2 Last element */
                if ((m2last->next == NULL) || (m2last->next == m2)) {
                    m2last->next = m1;
                    return;
                }
            
                /* Navigate forward */
                m2last = m2last->next;
            }
            return;
        }
        
        /* Try next monad */
        m1last = m1last->next;
    }
}


/** 
  Close (Loop) the monad chain m1.

  Syntactic sugar for monad_bind(m2, m1) if m1 is the first monad in the 
  chain and m2 is the last element.
    
  If the m1 is already a closed/looped monad chain, then 1 will be returned.
  Otherwise the returned value will be zero.
*/
int monad_loop(struct monad *m1) {
    struct monad *last = m1;
    while (true) {
        if (last->next == NULL) {
            /* Last element */
            last->next = m1;
            return OK;
        }

        if (last->next == m1) {
            /* It's already a closed loop, Do nothing. */
            return ERR;
        }

        last = last->next;
    }
}


/** 
  Make monad from task and bind it to m1.

  m1    task   result

  o-o   O      o-o-O 

  o-o   O      o-o-O
  |_|          |___|

*/
struct monad * monad_append(struct monad *m, monad_task task, void* args) {
    struct monad *newmonad = monad_return(task, args);
    monad_bind(m, newmonad);
    return newmonad;
}


void monad_failed(struct monad_context* ctx, const char *reason) {
    if (ctx->fail != NULL) {
        ctx->fail(ctx, (char *)reason);
    }
    free(ctx);
}


void monad_succeeded(struct monad_context* ctx, void *result) {
    struct monad *next = ctx->monad->next;
    if (next == NULL) {
        if (ctx->success != NULL) {
            ctx->success(ctx, result);
        }
        free(ctx);
        return;
    }

    ctx->monad = next;
    _run(ctx, next, result);
}


void monad_pass(struct monad_context *ctx, void *args, void *data) {
    monad_succeeded(ctx, data);
}


struct monad * monad_return(monad_task task, void *args) {
    struct monad *m = malloc(sizeof(struct monad));
    m->run = task;
    m->args = args;
    m->next = NULL;
    return m;
};


struct monad * monad_new() {
    return monad_return(monad_pass, NULL); 
};


static void _monad_free(struct monad *first, struct monad *m) {
    if (m == NULL) {
        return;
    }
    
    struct monad * next = m->next;
    free(m);

    if (next == first) {
        /* Disposition comppleted. */
        return;
    }
    
    _monad_free(first, next);
}


void monad_free(struct monad *m) {
    if (m == NULL) {
        return;
    }

    _monad_free(m, m);
}
