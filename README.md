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

## Coding Convention

- Opaque objects: PascalCase
- C Macros: UPPER_CASE
- Monads: fooM
- Monad factories: fooF
- All others: lowercase
