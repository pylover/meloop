CC = gcc
CFLAGS = \
	-I. \
	-D_GNU_SOURCE=1

LDFLAGS =


# Internal variables
objects = $(patsubst %.c,%.o,$(wildcard *.c))


# Executable
shell: shell.o $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)


shell.o: shell.c monad.h monad_io.h
	$(CC) -c $(CFLAGS) $< -o $@ 


# Implicit rule for other modules
%.o: %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@ 


.PHONY: clean
clean:: 
	- rm shell 
	- rm *.o


.PHONY: run-shell
run-shell: shell
	./shell
