CFLAGS = \
	-I.. \
	-g \
	-fms-extensions \
	-D_GNU_SOURCE=1

# LDFLAGS = -pg
objects = $(patsubst %.c,%.o,$(wildcard ../meloop/*.c))


# all: echo_client
all: state echoshell echoserver


%: %.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


echoshell: echoshell.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


echoserver: echoserver.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


echo_client: echo_client.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: clean
clean:: 
	- rm echoshell 
	- rm echoserver 
	- rm state 
	- rm massif.out.*
	- rm gmon.out
	- rm *.o
	cd ..; make clean


.PHONY: debug-echoshell
debug-echoshell: echoshell
	gdb ./$^


.PHONY: valgrind
valgrind: all
	valgrind \
		--tool=memcheck \
		--leak-check=full \
		--track-origins=yes \
		--gen-suppressions=all \
		./echoserver

# --suppressions=.valgrind.supp \
# .valgrind.supp file:
# {
#    io_epoll_ctl
#    Memcheck:Param
#    epoll_ctl(event)
#    fun:epoll_ctl
#    fun:_arm
#    fun:monad_io_again
#    fun:acceptM
#    fun:monad_execute
#    fun:monad_succeeded
#    fun:listenM
#    fun:monad_execute
#    fun:monad_run
#    fun:monad_tcp_runserver
#    fun:main
# }
