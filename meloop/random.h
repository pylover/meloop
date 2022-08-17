#ifndef MELOOP_RANDOM_H
#define MELOOP_RANDOM_H


struct randS {
    struct ioS;
    int fd;
};


void 
randopenA(struct circuitS *c, struct fileS *state, void *data);


void 
randreadA(struct circuitS *c, struct fileS *state, struct stringS *data);


void
randencA(struct circuitS *c, struct fileS *io, struct stringS *data);


#endif
