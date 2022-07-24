#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// MONAD_H Start

struct monad;
typedef void (*taskM) (struct monad *self, struct monad *x);


struct monad {
    taskM run;

    struct monad *next;
    bool last;
    bool done;
};


void 
runM (struct monad *self, struct monad *x) {
    self->run(self, x);
}


void 
resolveM(struct monad *self) {
    struct monad *next = self->next;
    if ((next == NULL) || self->nothing) {
        self->done = true;
    }
    else {
        next->run(next, self);
    }
}


struct monad * 
returnM (taskM task, struct monad *self) {
    self->run = task;
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
void 
bindM(struct monad *m1, struct monad *m2) {
    struct monad *m1last = m1;
    struct monad *m2last = m2;

    while (true) {
        /* Open cicuit */
        if (m1last->next == NULL) {
            /* m1 Last element */
            m1last->last = false;
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
                    m2last->last = true;
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
  Make monad from task and bind it to m1.

  m1    task   result

  o-o   O      o-o-O 

  o-o   O      o-o-O
  |_|          |___|

*/
struct monad * 
appendM(struct monad *self, taskM task, struct monad *other) {
    struct monad *newmonad = returnM(task, self);
    bindM(self, newmonad);
    return newmonad;
}


#define RESOLVE_M(self) resolveM((struct monad*)(self))
#define RUN_M(self, other) runM((struct monad*)(self), (struct monad*)(other))
#define RETURN_M(task, self) returnM((taskM)(task), (struct monad*)(self))
#define APPEND_M(self, task, other) \
    appendM((struct monad *)(self), (taskM)(task), (struct monad*)(other))


// MONAD_H End


struct maybe {
    struct monad;
    int just;
    bool nothing;
};


void addM(struct maybe *self, struct maybe *x) {
    self->just += x->just;
    RESOLVE_M(self); 
}


int main() {
    struct maybe m2 = {.run = addM, .just = 2};
    struct maybe m1 = {.run = addM, .just = 1, .next = &m2};
    struct maybe mz = {.just = 3};

    m1.run(&m1, &mz);

    
    printf("Result: %d\n", m1.just);
    return EXIT_SUCCESS;
}
