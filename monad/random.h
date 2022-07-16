#ifndef MONAD_RANDOM_H
#define MONAD_RANDOM_H


struct rand_props {
    struct io_props;
    int fd;
};


void 
urandom_openM(MonadContext *ctx, struct rand_props *props, struct conn *c);

void 
urandomM(MonadContext *ctx, struct rand_props *props, struct conn *c);


#endif
