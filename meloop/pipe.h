#ifndef MELOOP_PIPE_H
#define MELOOP_PIPE_H


#include "meloop/io.h"


struct pipeS {
    void *ptr;
    struct stringS *data;
    int rfd;
    int wfd;
};


void
pipereadA(struct circuitS *c, void *state, struct pipeS *pipe, 
        struct ioP *priv);


void
pipewriteA(struct circuitS *c, void *state, struct pipeS *pipe, 
        struct ioP *priv);


#endif
