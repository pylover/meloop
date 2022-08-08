#include "meloop/timer.h"


#include <sys/timerfd.h>


void
timeropenA(struct circuitS *c, struct ioS *s, struct stringS d) {
    struct timerS *t = (struct timerS*) meloop_priv_ptr(c);
    int fd = timerfd_create(t->clockid, t->flags | TFD_NONBLOCK);
    if (fd < 0) {
        ERROR_A(c, s, d, "timerfd_create");
        return;
    }
    
    t->fd = fd;
    RETURN_A(c, s, d);
}
