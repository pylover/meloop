#ifndef ARROW_TYPES_H
#define ARROW_TYPES_H


struct pair {
    int left;
    int right;
};


union args {
    int sint;
    struct pair pair;
};


typedef union args (*arrow) (void* state, union args args);


#endif
