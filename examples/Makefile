CFLAGS = -I..
objects = $(patsubst %.c,%.o,$(wildcard ../monad/*.c))


echo_shell: echo_shell.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


echo_server: echo_server.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: clean
clean:: 
	- rm echo_shell 
	- rm echo_server 
	cd ..; make clean


.PHONY: debug-echo_shell
debug-echo_shell: echo_shell
	gdb ./$^
