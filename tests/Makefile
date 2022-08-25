SRC = ../meloop
CFLAGS = \
	-I.. \
	-g \
	-fms-extensions \
	-D_GNU_SOURCE=1
	# -fprofile-arcs \
	# -ftest-coverage \


LDFLAGS = `pkg-config --libs openssl`


objects = $(patsubst %.c,%.o,$(wildcard $(SRC)/*.c))
execs = \
	test_circuit_simple \
	test_circuit_loop


all: $(execs)


# Implicit rule for meloop modules
$(SRC)/%.o: $(SRC)/%.c $(SRC)/%.h
	$(CC) -c $(CFLAGS) $< -o $@ 


$(execs):%: %.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 


.PHONY: clean
clean:: 
	- rm *.gcov *.gcda *.gcno
	- rm *.o
	- rm $(execs) 
	- rm massif.out.*
	- rm gmon.out
	cd ..; make clean
