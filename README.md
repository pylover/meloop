# meloop (Monadic Event Loop)


## Build


```bash
make clean
make all
make clean all
```


## Quickstart

### Simple TCP Server

```c
#include "monad/tcp.h"


void main() {
    monad_io_init(0);
    static struct io_props client_props = {
        .epollflags = EPOLLET, 
        .readsize = 1024
    };
    
    struct bind bindinfo = {
        .host = "127.0.0.1",
        .port = 9090,
        .client_monad = echoloopF(&client_props),
    };
    
    monad_tcp_runserver(&bindinfo, NULL, NULL);
    monad_free(bindinfo.client_monad);
    monad_io_deinit();
}
```

## Examples

```bash
cd examples

make echo_shell
./echo_shell


make echo_server
./echo_server
```

## Profiling


### gprof

Run desired example, signal it with `CTRL+C`, then:

```bash
cd examples

make echo_server
./echo_server

gprof echo_server
```

### valgrind

```bash
sudo apt install valgrind
make clean all 
valgrind --tool=memcheck --leak-check=full ./echo_server
valgrind --tool=massif ./echo_server 
```

Example output

```bash
==56319== Memcheck, a memory error detector
==56319== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==56319== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==56319== Command: ./echo_server
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
