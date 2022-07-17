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
resolveM(struct monad *self) {
    self->done = true;
}


void 
runM (struct monad *self, struct monad *x) {
    self->run(self, x);
}


struct monad * 
returnM (taskM task, struct monad *self) {
    self->run = task;
}


#define RESOLVE_M(self) resolveM((struct monad*)(self))
#define RUN_M(self, other) runM((struct monad*)(self), (struct monad*)(other))
#define RETURN_M(task, self) returnM((taskM)(task), (struct monad*)(self))


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
    struct maybe m1 = {.just = 2};
    struct maybe m2 = {.just = 3};

    struct monad *all = RETURN_M(addM, &m1);

    RUN_M(all, &m2);
    
    printf("Result: %d\n", m1.just);
    return EXIT_SUCCESS;
}
