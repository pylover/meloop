
ET / LT
Non-Blocking / Blocking


a) with nonblocking file descriptors; and

b) by waiting for an event only after read(2) or write(2) return EAGAIN.


- Document every function
- SOCK_NONBLOCK
- EPOLLET
- EAGAIN

- Better naming 
- Null error handler for all mallocs
- Monad loop
- Test for all functions
