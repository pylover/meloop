#ifndef MELOOP_RANDOM_H
#define MELOOP_RANDOM_H


struct rand {
    struct io;
    int randfd;
};


void 
random_openA(struct circuit *c, struct rand *state, struct string d);


void 
random_readA(struct circuit *c, struct rand *state, struct string d);


#endif
