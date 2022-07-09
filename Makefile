CC = gcc
CFLAGS = \
	-I. \
	-D_GNU_SOURCE=1

LDFLAGS =


# Internal variables
common_headers = 
objects = $(patsubst %.c,%.o,$(wildcard *.c))


# Executable
main: main.o $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)


main.o: main.c $(common_headers)
	$(CC) -c $(CFLAGS) $< -o $@ 


# Implicit rule for other modules
%.o: %.c %.h $(common_headers)
	$(CC) -c $(CFLAGS) $< -o $@ 


.PHONY: clean
clean:: 
	- rm main 
	- rm *.o


.PHONY: run
run: main
	./main
