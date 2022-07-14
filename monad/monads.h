#ifndef MONAD_MONADS_H
#define MONAD_MONADS_H


#include "monad.h"
#include "io.h"
#include "tcp.h"


/* Monads */
void passM(MonadContext *, void *args, void *data);


/* IO Monads */
void waitrM(MonadContext *ctx, struct device *dev, struct conn *c); 
void waitwM(MonadContext *ctx, struct device *dev, struct conn *c);
void writeM(MonadContext *ctx, struct device *dev, struct conn *c);
void readM(MonadContext *ctx, struct device *dev, struct conn *c);


/* TCP Server Monads */
void listenM(MonadContext *ctx, struct device *dev, struct conn *c);
void acceptM(MonadContext *ctx, struct device *dev, struct conn *c);


/* Monad Factories */
Monad * echoF(struct device *dev);


#endif
