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
timeropenA(struct circuitS *c, void *global, struct timerS *t, 
        struct timerP *priv) {
    struct itimerspec ispec;
    time_t sec = priv->interval_ns / NS;
    long nsec = (long) (priv->interval_ns % NS);
    int fd = timerfd_create(priv->clockid, priv->flags | TFD_NONBLOCK);

    if (fd < 0) {
        ERROR_A(c, global, t, "timerfd_create");
        return;
    }
    
    t->fd = fd;
    ispec.it_interval.tv_sec = sec; 
    ispec.it_interval.tv_nsec = nsec; 
    ispec.it_value.tv_sec = sec; 
    ispec.it_value.tv_nsec = nsec; 

    if (timerfd_settime(fd, 0, &ispec, NULL)) {
        close(fd);
        ERROR_A(c, global, t, "timerfd_settime");
        return;
    }
    t->status = IDLE;        
    RETURN_A(c, global, t);
}


void 
timersleepA(struct circuitS *c, void *global, struct timerS *t, 
        struct timerP *priv) {
    ssize_t size;
    uint64_t res;

    if (t->status == IDLE) {
        WAIT_A(c, global, t, t->fd, EPOLLIN, priv->epollflags);
        t->status = WAITING;
        return;
    }

    /* Read from the file descriptor */
    size = read(t->fd, &res, sizeof(res));
    //printf("timer: readed: %lu %lu\n", size, res);

    /* Check for EOF */
    if (size == 0) {
        ERROR_A(c, global, t, "EOF");
        return;
    }

    /* Error | wait */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, global, t, t->fd, EPOLLIN, priv->epollflags);
        }
        else {
            ERROR_A(c, global, t, "read timerfd");
        }
        return;
    }
    RETURN_A(c, global, t);
}
