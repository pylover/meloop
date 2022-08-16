#include "meloop/timer.h"
#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>


#define NS  1000000000
#define IDLE    0
#define WAITING 1


void
timeropenA(struct circuitS *c, struct fileS *s, struct stringS d) {
    struct itimerspec ispec;
    struct timerS *t = (struct timerS*) meloop_priv_ptr(c);
    time_t sec = t->interval_ns / NS;
    long nsec = (long) (t->interval_ns % NS);
    int fd = timerfd_create(t->clockid, t->flags | TFD_NONBLOCK);

    if (fd < 0) {
        ERROR_A(c, s, d, "timerfd_create");
        return;
    }
    
    t->fd = fd;
    ispec.it_interval.tv_sec = sec; 
    ispec.it_interval.tv_nsec = nsec; 
    ispec.it_value.tv_sec = sec; 
    ispec.it_value.tv_nsec = nsec; 

    if (timerfd_settime(fd, 0, &ispec, NULL)) {
        close(fd);
        ERROR_A(c, s, d, "timerfd_settime");
        return;
    }
    t->status = IDLE;        
    RETURN_A(c, s, d);
}


void 
timersleepA(struct circuitS *c, struct fileS *s, struct stringS d) {
    struct timerS *t = (struct timerS*) meloop_priv_ptr(c);
    ssize_t size;
    uint64_t res;

    if (t->status == IDLE) {
        WAIT_A(c, s, d, t->fd, EPOLLIN);
        t->status = WAITING;
        return;
    }

    /* Read from the file descriptor */
    size = read(t->fd, &res, sizeof(res));
    //printf("timer: readed: %lu %lu\n", size, res);

    /* Check for EOF */
    if (size == 0) {
        ERROR_A(c, s, d, "EOF");
        return;
    }

    /* Error | wait */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, d, t->fd, EPOLLIN);
        }
        else {
            ERROR_A(c, s, d, "read timerfd");
        }
        return;
    }
    RETURN_A(c, s, d);
}
