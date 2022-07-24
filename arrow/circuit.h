#ifndef ARROW_CIRCUIT_H
#define ARROW_CIRCUIT_H


#include "arrow/types.h"


typedef union args (*computation) (void* state, union args args);
struct circuit;


struct circuit * 
returnC(computation f);


void 
freeC(struct circuit *c);


void 
bindA(struct circuit *c1, struct circuit *c2);


struct circuit * 
appendC(struct circuit *c1, computation f);


int 
loopC(struct circuit *c1);


#endif
