#ifndef IO_H
#define IO_H


typedef struct io IO;
typedef struct io_monad IOMonad;


typedef void (*io_task) (IO*, void *);


IOMonad * io_new();
IOMonad * io_return(io_task);
void io_pass(IO *io, void *a);
void io_succeeded(IO*, void *);
void io_failed(IO*, const char *);


void io_bind(IOMonad*, IOMonad*);
void io_append(IOMonad*, io_task);
void io_run(IOMonad*, void *input, io_task success, io_task fail);


#endif
