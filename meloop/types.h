#ifndef MELOOP_TYPES_H
#define MELOOP_TYPES_H


#include <string.h>


#define ERR -1
#define OK 0


struct pairS {
    int left;
    int right;
};


struct stringS {
    size_t size;
    char *data;
};


union any {
    int sint;
    void *ptr;
    struct pairS pair;
    struct stringS string;
};


struct stringS 
meloop_atos(char *s);


#endif
