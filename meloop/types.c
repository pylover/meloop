#include "meloop/types.h"


struct string 
string_from_char(char *s) {
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



union any
any_null() {
    union any out = {
        .ptr = NULL
    };

    return out;
}


union any
any_string(struct string s) {
    union any out = {
        .string = s
    };

    return out;
}
