#include "meloop/types.h"


struct string 
meloop_atos(char *s) {
    struct string out = {
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
