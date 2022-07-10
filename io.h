#ifndef IO_H
#define IO_H


typedef struct io IO;
typedef struct io_monad IOMonad;


typedef void (*io_task) (IO*, void *);
typedef void (*io_success_callback) (void *);
typedef void (*io_fail_callback) (const char *);


IOMonad * io_new(io_task);
void io_succeeded(IO*, void *);
void io_failed(IO*, const char *);


void io_bind(IOMonad*, IOMonad*);
void io_append(IOMonad*, io_task);
void io_run(IOMonad*, void *input, io_success_callback, io_fail_callback);


#endif
