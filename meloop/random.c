#include <meloop/arrow.h>
#include <meloop/io.h>
#include <meloop/random.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <linux/random.h>


void 
randopenA(struct circuitS *c, struct fileS *s, struct stringS d) {
    struct randS *r = (struct randS*) meloop_priv_ptr(c);
    int fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        ERROR_A(c, s, d, "open urandom");
        return;
    }
    r->fd = fd;
    RETURN_A(c, s, d);
}


void 
randreadA(struct circuitS *c, struct fileS *s, struct stringS d) {
    struct randS *r = (struct randS*) meloop_priv_ptr(c);
    size_t size;

    /* Read from the file descriptor */
    size = read(r->fd, d.data, s->readsize);

    /* Check for error */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, d, r->fd, EPOLLIN);
        }
        else {
            ERROR_A(c, s, d, "read urandom");
        }
        return;
    }
    d.size = size;
    RETURN_A(c, s, d);
}


void
randencA(struct circuitS *c, struct fileS *io, struct stringS buff) {
    int i;
    unsigned int t;
    for (i = 0; i < buff.size; i++) {
        t = buff.data[i];
        buff.data[i] = (t % 26) + 97;
    }
    RETURN_A(c, io, buff);
}
