#ifndef MELOOP_RANDOM_H
#define MELOOP_RANDOM_H


struct randS {
    struct ioS;
    int randfd;
};


void 
randopenA(struct circuitS *c, struct randS *state, struct stringS d);


void 
randreadA(struct circuitS *c, struct randS *state, struct stringS d);


#endif
