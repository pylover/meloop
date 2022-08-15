# meloop (Monadic Event Loop)


## Features

- Async/non-blocking IO
- IO: read, write, wait(using epoll)
- TCP client: connectA
- TCP server: listenA, acceptA
- SSL/TLS: tlsA, tlsreadA, tlswriteA
- Timer using linux timerfd: timeropenA, timersleepA
- Random using /dev/urandom: randopenA, randreadA, randencA

## Build


### Dependencies

```bash
sudo apt install build-essential pkgconf openssl
```


```bash
make clean
make all
make clean all
```

## Examples

Take a look at `examples` directory.

```bash
cd examples

make echoshell
./echoshell


make echoserver
./echoserver


make echoclient
./echoclient


make tlsclient
./tlsclient


make random
./random


make state
./state
```


## Profiling


### gprof

Run desired example, signal it with `CTRL+C`, then:

```bash
cd examples

make echoserver
./echoserver

gprof echoserver
```

### valgrind

```bash
sudo apt install valgrind

cd examples
make clean all 
make valgrind
```

Or

```bash
valgrind --tool=memcheck --leak-check=full ./echoserver
valgrind --tool=massif ./echoserver 
```

Example output

```bash
==56319== Memcheck, a memory error detector
==56319== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==56319== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==56319== Command: ./echoserver
==56319==
127.0.0.1:42658 Connected.
127.0.0.1:42658 Disconnected.
^C
SIGINT detected: 2
Terminating TCP Server
==56319==
==56319== HEAP SUMMARY:
==56319==     in use at exit: 0 bytes in 0 blocks
==56319==   total heap usage: 14 allocs, 14 frees, 2,384 bytes allocated
==56319==
==56319== All heap blocks were freed -- no leaks are possible
==56319==
==56319== For lists of detected and suppressed errors, rerun with: -s
==56319== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```


## Coding Convention

- Opaque objects: PascalCase
- C Macros: UPPER_CASE
- Monads: fooM
- Monad factories: fooF
- All others: lowercase
