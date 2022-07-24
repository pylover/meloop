#ifndef ARROW_TYPES_H
#define ARROW_TYPES_H


struct pair {
    int left;
    int right;
};


union args {
    int sint;
    struct pair;
};


#endif
