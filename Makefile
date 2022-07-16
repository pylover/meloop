CC = gcc
CFLAGS = \
	-I. \
	-fms-extensions \
	-D_GNU_SOURCE=1

# Uncomment for production
CFLAGS += -g


# Internal variables
objects = $(patsubst %.c,%.o,$(wildcard monad/*.c))


.PHONY: all
all: $(objects)


# Implicit rule for other modules
%.o: %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@ 


.PHONY: clean
clean:: 
	- rm monad/*.o
