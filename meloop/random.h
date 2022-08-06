#ifndef MELOOP_RANDOM_H
#define MELOOP_RANDOM_H


struct rand {
    struct io;
    int randfd;
};


void 
randopenA(struct circuit *c, struct rand *state, struct string d);


void 
randreadA(struct circuit *c, struct rand *state, struct string d);


#endif
