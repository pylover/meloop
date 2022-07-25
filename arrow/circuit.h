#ifndef ARROW_CIRCUIT_H
#define ARROW_CIRCUIT_H


#include "arrow/types.h"


struct circuit;


struct circuit * 
returnC(arrow f);


void 
freeC(struct circuit *c);


void 
bindA(struct circuit *c1, struct circuit *c2);


struct circuit * 
appendC(struct circuit *c1, arrow f);


int 
loopC(struct circuit *c1);


union args runA(struct circuit *c, void *state, union args);


#endif
