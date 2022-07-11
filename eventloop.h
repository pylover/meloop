// #ifndef EVENTLOOP_H
// #define EVENTLOOP_H
// 
// 
// #include "common.h"
// 
// 
// typedef struct ev_bag ev_bag;
// 
// 
// enum ev_event {
//     EV_READ,
//     EV_WRITE
// };
// 
// 
// typedef void (*ev_callback) (void *ptr);
// 
// 
// struct ev_bag {
//     ev_callback callback;
//     void *ptr;
// };
// 
// 
// void ev_init();
// void ev_deinit();
// void ev_wait();
// err_t ev_rearm(int fd, enum ev_event e, ev_callback cb, void *ptr);
// err_t ev_arm(int fd, enum ev_event e, ev_callback cb, void *ptr);
// err_t ev_dearm(int fd);
// 
// 
// #endif
