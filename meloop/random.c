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
randopenA(struct circuitS *c, void *s, struct stringS *data, 
        struct randP *priv) {
    int fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        ERROR_A(c, s, data, "open urandom");
        return;
    }
    priv->fd = fd;
    RETURN_A(c, s, data);
}


void 
randreadA(struct circuitS *c, void *s, struct stringS *data,
        struct randP *priv) {
    size_t size;

    /* Read from the file descriptor */
    size = read(priv->fd, data->buffer, priv->readsize);

    /* Check for error */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, data, priv->fd, EPOLLIN, priv->epollflags);
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
