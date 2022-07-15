# meloop (Monadic Event Loop)


## Build


```bash
make clean
make all
make clean all
```


### Examples

```bash
cd examples

make echo_shell
./echo_shell


make echo_server
./echo_server
```

#### Profiling


##### gprof

Run desired example, signal it with `CTRL+C`, then:

```bash
cd examples

make echo_server
./echo_server

gprof echo_server
```

##### valgrind

```bash
sudo apt install valgrind
make clean all 
valgrind --tool=memcheck --leak-check=full ./echo_server
valgrind --tool=massif ./echo_server 
```


## Coding Convention

- Opaque objects: PascalCase
- C Macros: UPPER_CASE
- Monads: fooM
- Monad factories: fooF
- All others: lowercase
