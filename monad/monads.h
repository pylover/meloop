#ifndef MONAD_MONADS_H
#define MONAD_MONADS_H


#include "monad.h"
#include "io.h"
#include "tcp.h"


/* Monads */
void passM(MonadContext *, void *args, void *data);


/* IO Monads */
void awaitrM(MonadContext *ctx, struct device *dev, struct conn *c); 
void awaitwM(MonadContext *ctx, struct device *dev, struct conn *c);

void writerM(MonadContext *ctx, struct device *dev, struct conn *c);
void readerM(MonadContext *ctx, struct device *dev, struct conn *c);

struct monad * readerF(struct device *dev);
struct monad * writerF(struct device *dev); 


/* TCP Server Monads */
void listenM(MonadContext *ctx, struct device *dev, struct conn *c);
void acceptM(MonadContext *ctx, struct device *dev, struct conn *c);


/* Monad Factories */
Monad * echoF(struct device *dev);


#endif
