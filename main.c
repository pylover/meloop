#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <err.h>


#define BUFFSIZE    1024


typedef void (*io_callback) (void *);


struct packet {
  char *value;
  size_t len;
};


typedef struct IO {
  struct packet *packet;
} IO;


typedef IO * (*doIO) (struct packet *);


IO * returnIO (struct packet *p) {
    IO * m = malloc(sizeof(IO));
    m->packet = p;
    return m;
}


IO * readlineIO () {
    struct packet * p = malloc(sizeof(struct packet));
    p->value = NULL;
    p->len = 0;

    ssize_t res = getline(&(p->value), &(p->len), stdin);
    if (res < 0) {
        err(1, "getline(stdin)");
    }
    p->len = res;
    return returnIO(p);
}


IO * writeIO (struct packet *p) {
    if (write(STDOUT_FILENO, p->value, p->len) < 0) {
        err(1, "write(stdout)");
    }
    return returnIO(NULL);
}


IO * bind(IO *a, doIO b) {
    if (a->packet == NULL) {
        return returnIO(NULL);
    }

    return b(a->packet);
}


int main() {
    IO * in = readlineIO();
    IO * m = bind(in, writeIO);
    return 0;
}
