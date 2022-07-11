// #include "eventloop.h"
// 
// #include <unistd.h>
// #include <sys/epoll.h>
// 
// 
// #define EPOLLCTL_FLAGS (EPOLLET | EPOLLONESHOT | EPOLLRDHUP | EPOLLERR)
// #define EV_TYPE (ev) ((ev.events & EPOLLIN == EPOLLIN)? EV_READ: EV_WRITE)
// 
// 
// 
// static err_t _ev_arm(int fd, int op, enum ev_event e, ev_callback cb, 
//         void *ptr) {
//     struct ev_bag *bag;
//     struct epoll_event ev;
//     
//     /* Allocate for event */
//     bag = malloc(sizeof (struct ev_bag));
//     if (bag == NULL) {
//         FATAL("Out of memory");
//     }
//     bag->callback = cb;
//     bag->ptr = ptr;
//     
//     /* Read / Write */
//     if (e == EV_READ) {
//         ev.events = EPOLLIN;
//     }
//     else {
//         ev.events = EPOLLOUT;
//     }
// 
//     ev.events += EPOLLCTL_FLAGS;
//     ev.data.ptr = bag;
//     if (epoll_ctl(state.epoll_fd, op, fd, &ev) != OK) {
//         if (errno == ENOENT) {
//             return ev_arm(fd, e, cb, ptr);
//         }
//         return ERR;
//     }
//     return OK;
// }
// 
// 
// err_t ev_rearm(int fd, enum ev_event e, ev_callback cb, void *ptr) {
//     if (_ev_arm(fd, EPOLL_CTL_MOD, e, cb, ptr) != OK) {
//         if (errno == ENOENT) {
//             return ev_arm(fd, e, cb, ptr);
//         }
//     }
//     return OK;
// }
// 
// 
// err_t ev_arm(int fd, enum ev_event e, ev_callback cb, void *ptr) {
//     return _ev_arm(fd, EPOLL_CTL_ADD, e, cb, ptr);
// }
// 
// 
