#ifndef MELOOP_TYPES_H
#define MELOOP_TYPES_H


#include <string.h>


#define ERR -1
#define OK 0


struct pair {
    int left;
    int right;
};


struct string {
    size_t size;
    char *data;
};


union any {
    int sint;
    char *charptr;
    void *ptr;
    struct pair pair;
    struct string string;
};


struct string 
meloop_atos(char *s);


#endif
