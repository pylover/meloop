#ifndef IO_H
#define IO_H


typedef struct io IO;
typedef struct io_monad IOMonad;


typedef void (*io_task) (IO*, void* args, void *data);
typedef void (*io_success) (IO*, void *data);
typedef void (*io_failure) (IO*, const char *reason);


void io_free(IOMonad *);
IOMonad * io_new();
IOMonad * io_return(io_task, void*);
void io_pass(IO *io, void *args, void *data);
void io_succeeded(IO*, void *data);
void io_failed(IO*, const char *);


void io_bind(IOMonad*, IOMonad*);
void io_append(IOMonad*, io_task, void*);
void io_run(IOMonad*, void *input, io_success success, io_failure fail);


#define IO_RETURN(t, a) io_return((io_task)(t), a)
#define IO_APPEND(m, t, a) io_append(m, (io_task)(t), a)
#endif
