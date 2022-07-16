#include <monad/io.h>
#include <monad/random.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <linux/random.h>


void 
urandom_openM(MonadContext *ctx, struct rand_props *props, 
        struct conn *c) {
    int fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        monad_failed(ctx, c, "open urandom");
        return;
    }
    props->fd = fd;
    monad_succeeded(ctx, c);
}


void 
urandomM(MonadContext *ctx, struct rand_props *props, struct conn *c) {
    size_t size;

    /* Read from the file descriptor */
    size = read(props->fd, c->data, props->readsize);

    /* Check for error */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            MONAD_IO_AGAIN(ctx, props, c, EPOLLIN);
        }
        else {
            monad_failed(ctx, c, "read");
        }
        return;
    }
    c->size = size;
    monad_succeeded(ctx, c);
}
