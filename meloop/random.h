#ifndef MELOOP_RANDOM_H
#define MELOOP_RANDOM_H


struct randS {
    struct ioS;
    int fd;
};


void 
randopenA(struct circuitS *c, struct fileS *state, struct stringS d);


void 
randreadA(struct circuitS *c, struct fileS *state, struct stringS d);


void
randencA(struct circuitS *c, struct fileS *io, struct stringS buff);


#endif
