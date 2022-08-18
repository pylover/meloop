#ifndef MELOOP_RANDOM_H
#define MELOOP_RANDOM_H


struct randS {
    struct ioS;
    int fd;
};


void 
randopenA(struct circuitS *c, void *s, struct stringS *data);


void 
randreadA(struct circuitS *c, void *s, struct stringS *data);


void
randencA(struct circuitS *c, void *s, struct stringS *data);


#endif
