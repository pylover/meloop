CC = gcc
CFLAGS = \
	-I. \
	-D_GNU_SOURCE=1

LDFLAGS =


# Internal variables
common_headers = 
objects = $(patsubst %.c,%.o,$(wildcard *.c))


# Executable
shell: shell.o $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)


shell.o: shell.c $(common_headers)
	$(CC) -c $(CFLAGS) $< -o $@ 


# Implicit rule for other modules
%.o: %.c %.h $(common_headers)
	$(CC) -c $(CFLAGS) $< -o $@ 


.PHONY: clean
clean:: 
	- rm shell 
	- rm *.o


.PHONY: run-shell
run-shell: shell
	./shell
