SRC = ../meloop
CFLAGS = \
	-I.. \
	-g \
	-fms-extensions \
	-D_GNU_SOURCE=1


LDFLAGS = `pkg-config --libs openssl`


objects = $(patsubst %.c,%.o,$(wildcard $(SRC)/*.c))
# execs = echoshell echoserver echoclient tlsclient random
execs = random echoserver echoclient tlsclient


all: $(execs)


# Implicit rule for meloop modules
$(SRC)/%.o: $(SRC)/%.c $(SRC)/%.h
	$(CC) -c $(CFLAGS) $< -o $@ 


$(execs):%: %.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 


.PHONY: clean
clean:: 
	- rm echoshell 
	- rm echoserver 
	- rm echoclient
	- rm tlsclient
	- rm state 
	- rm random 
	- rm massif.out.*
	- rm gmon.out
	- rm *.o
	cd ..; make clean


.PHONY: debug-echoshell
debug-echoshell: echoshell
	gdb ./$^


.PHONY: valgrind-%
valgrind-%: %
	valgrind \
		--tool=memcheck \
		--leak-check=full \
		--track-origins=yes \
		--gen-suppressions=all \
		./$^

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
