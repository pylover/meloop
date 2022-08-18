#include <meloop/arrow.h>
#include <meloop/io.h>
#include <meloop/random.h>
#include <meloop/logging.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <linux/random.h>


void 
randopenA(struct circuitS *c, void *s, struct stringS *data) {
    struct randS *r = (struct randS*) meloop_priv_ptr(c);
    int fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        ERROR_A(c, s, data, "open urandom");
        return;
    }
    r->fd = fd;
    RETURN_A(c, s, data);
}


void 
randreadA(struct circuitS *c, void *s, struct stringS *data) {
    struct randS *r = (struct randS*) meloop_priv_ptr(c);
    size_t size;

    /* Read from the file descriptor */
    size = read(r->fd, data->buffer, r->readsize);

    /* Check for error */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, data, r->fd, EPOLLIN);
        }
        else {
            ERROR_A(c, s, data, "read urandom");
        }
        return;
    }
    data->size = size;
    RETURN_A(c, s, data);
}


void
randencA(struct circuitS *c, void *s, struct stringS *data) {
    int i;
    unsigned int t;
    for (i = 0; i < data->size; i++) {
        t = data->buffer[i];
        data->buffer[i] = (t % 26) + 97;
    }
    RETURN_A(c, s, data);
}
