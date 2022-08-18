#ifndef MELOOP_PIPE_H
#define MELOOP_PIPE_H


#include "meloop/io.h"


struct pipeS {
    struct stringS;
    int rfd;
    int wfd;
};


void
pipereadA(struct circuitS *c, void *state, struct pipeS *pipe);


void
pipewriteA(struct circuitS *c, void *state, struct pipeS *pipe);


#endif
