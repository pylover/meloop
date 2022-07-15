#ifndef MONAD_H
#define MONAD_H


typedef struct monad_context MonadContext;
typedef struct monad Monad;


typedef void (*monad_task) (MonadContext*, void* args, void *data);
typedef void (*monad_finish) (MonadContext*, void *data, const char *reason);


void * monad_args(struct monad *m);
void monad_free(Monad *);
Monad * monad_new();
Monad * monad_return(monad_task, void*);
void monad_succeeded(MonadContext*, void *data);
void monad_failed(struct monad_context* ctx, void *data, 
        const char *format, ...);

struct monad * monad_append(struct monad *, monad_task , void* );
void monad_bind(Monad*, Monad*);
int monad_loop(struct monad *m1);

void monad_execute(struct monad_context *ctx, void *data);
struct monad_context * monad_runall(struct monad *m, void *data, 
        monad_finish finish);
void monad_terminate(struct monad_context *ctx, void *data, 
        const char *reason);


/* Monads */
void passM(MonadContext *, void *args, void *data);


#define MONAD_RUNALL(m, d, s) monad_runall(m, d, (monad_finish)(s))
#define MONAD_RETURN(t, a) monad_return((monad_task)(t), a)
#define MONAD_APPEND(m, t, a) monad_append(m, (monad_task)(t), a)
#define MONAD_BIND(m1, m2) monad_bind(m1, m2)


#endif
