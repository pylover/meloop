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
randopenA(struct circuitS *c, void *s, struct fileS *f) {
    int fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        ERROR_A(c, s, f, "open urandom");
        return;
    }
    f->fd = fd;
    RETURN_A(c, s, f);
}


void 
randreadA(struct circuitS *c, void *s, struct fileS *f, struct ioP *priv) {
    size_t size;

    /* Read from the file descriptor */
    size = read(f->fd, f->data->blob, priv->readsize);

    /* Check for error */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, f, f->fd, EPOLLIN, priv->epollflags);
        }
        else {
            ERROR_A(c, s, f, "read urandom");
        }
        return;
    }
    f->data->size = size;
    RETURN_A(c, s, f);
}


void
randencA(struct circuitS *c, void *s, struct fileS *f) {
    int i;
    unsigned int t;
    for (i = 0; i < f->data->size; i++) {
        t = f->data->blob[i];
        f->data->blob[i] = (t % 26) + 97;
    }
    RETURN_A(c, s, f);
}
