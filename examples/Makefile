CFLAGS = -I..
objects = $(patsubst %.c,%.o,$(wildcard ../*.c))


# Executables
%: %.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: clean
clean:: 
	- rm echo_shell 
	- rm *.o


.PHONY: debug-echo_shell
debug-echo_shell: echo_shell
	gdb ./$^
