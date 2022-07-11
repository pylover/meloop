#ifndef MONAD_H
#define MONAD_H


typedef struct monad_context MonadContext;
typedef struct monad Monad;


typedef void (*monad_task) (MonadContext*, void* args, void *data);
typedef void (*monad_success) (MonadContext*, void *data);
typedef void (*monad_failure) (MonadContext*, const char *reason);


void monad_free(Monad *);
Monad * monad_new();
Monad * monad_return(monad_task, void*);
void monad_pass(MonadContext *, void *args, void *data);
void monad_succeeded(MonadContext*, void *data);
void monad_failed(MonadContext*, const char *);


struct monad * monad_append(struct monad *, monad_task , void* );
void monad_bind(Monad*, Monad*);
void monad_run(Monad*, void *input, monad_success, monad_failure);


#define MONAD_RETURN(t, a) monad_return((monad_task)(t), a)
#define MONAD_APPEND(m, t, a) monad_append(m, (monad_task)(t), a)
#define MONAD_BIND(m1, m2) monad_bind((m1), (m2))


#endif
