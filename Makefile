CC = gcc
CFLAGS = \
	-I. \
	-fms-extensions \
	-D_GNU_SOURCE=1

# Uncomment for production
CFLAGS += -g


# Internal variables
objects = $(patsubst %.c,%.o,$(wildcard meloop/*.c))


.PHONY: all
all: $(objects)


# Implicit rule for modules
%.o: %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@ 


.PHONY: clean
clean:: 
	- rm meloop/*.o
