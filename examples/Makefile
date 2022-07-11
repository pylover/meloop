CFLAGS = -I..
objects = $(patsubst %.c,%.o,$(wildcard ../*.c))


echo_shell: echo_shell.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: clean
clean:: 
	- rm echo_shell 
	cd ..; make clean


.PHONY: debug-echo_shell
debug-echo_shell: echo_shell
	gdb ./$^
