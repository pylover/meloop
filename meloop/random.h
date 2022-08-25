#ifndef MELOOP_RANDOM_H
#define MELOOP_RANDOM_H


struct randP {
    struct ioP;
    int fd;
};


void 
randopenA(struct circuitS *c, void *s, struct stringS *data, 
        struct randP *priv);


void 
randreadA(struct circuitS *c, void *s, struct stringS *data,
        struct randP *priv);


void
randencA(struct circuitS *c, void *s, struct stringS *data);


#endif
