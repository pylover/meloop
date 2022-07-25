CFLAGS = \
	-I.. \
	-g \
	-fms-extensions \
	-D_GNU_SOURCE=1

# LDFLAGS = -pg
objects = $(patsubst %.c,%.o,$(wildcard ../arrow/*.c))


# all: maybe echo_shell echo_server echo_client
all: state


%: %.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


maybe: maybe.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


echo_shell: echo_shell.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


echo_server: echo_server.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


echo_client: echo_client.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: clean
clean:: 
	- rm echo_shell 
	- rm echo_server 
	- rm state 
	- rm maybe 
	- rm massif.out.*
	- rm gmon.out
	cd ..; make clean


.PHONY: debug-echo_shell
debug-echo_shell: echo_shell
	gdb ./$^


.PHONY: valgrind
valgrind: all
	valgrind \
		--tool=memcheck \
		--leak-check=full \
		--track-origins=yes \
		--suppressions=.valgrind.supp \
		--gen-suppressions=all \
		./echo_server

