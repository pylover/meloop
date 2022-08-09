#include "meloop/types.h"


struct stringS 
meloop_string(char *s) {
    struct stringS out = {
        .size = 0,
        .data = NULL
    };

    if (s == NULL) {
        return out;
    }

    out.size = strlen(s);
    out.data = s;
    return out;
}


void *
meloop_ptr(void *p) {
    return p;
}
