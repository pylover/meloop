#ifndef MELOOP_RANDOM_H
#define MELOOP_RANDOM_H


#include "meloop/io.h"


void 
randopenA(struct circuitS *c, void *s, struct fileS *f);


void 
randreadA(struct circuitS *c, void *s, struct fileS *f, struct ioP *priv);


void
randencA(struct circuitS *c, void *s, struct fileS *f);


#endif
