CFLAGS = \
	-I.. \
	-g \
	-D_GNU_SOURCE=1

LDFLAGS = -pg
objects = $(patsubst %.c,%.o,$(wildcard ../monad/*.c))


all: echo_shell echo_server echo_miniserver


echo_shell: echo_shell.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


echo_server: echo_server.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


echo_miniserver: echo_miniserver.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: clean
clean:: 
	- rm echo_shell 
	- rm echo_server 
	- rm echo_miniserver 
	cd ..; make clean


.PHONY: debug-echo_shell
debug-echo_shell: echo_shell
	gdb ./$^
