#ifndef ARROW_RANDOM_H
#define ARROW_RANDOM_H


struct rand{
    struct io;
};


void 
urandom_openA(struct circuit *c, struct rand *state, struct string d);


void 
urandom_readA(struct circuit *c, struct rand *state, struct string d);


#endif
