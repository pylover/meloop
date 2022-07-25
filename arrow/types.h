#ifndef ARROW_TYPES_H
#define ARROW_TYPES_H


#include <stdbool.h>


struct pair {
    int left;
    int right;
};


union args {
    bool null;
    int sint;
    struct pair pair;
};


typedef union args (*arrow) (void* state, union args value, union args priv);


#endif
