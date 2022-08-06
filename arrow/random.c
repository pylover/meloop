#include <arrow/arrow.h>
#include <arrow/io.h>
#include <arrow/random.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <linux/random.h>


void 
urandom_openA(struct circuit *c, struct rand *s, struct string d) {
    int fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        ERROR_A(c, s, d, "open urandom");
        return;
    }
    s->rfd = fd;
    RETURN_A(c, s, d);
}


void 
urandom_readA(struct circuit *c, struct rand *s, struct string d) {
    size_t size;

    /* Read from the file descriptor */
    size = read(s->rfd, d.data, s->readsize);

    /* Check for error */
    if (size < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            WAIT_A(c, s, d, s->rfd, EPOLLIN);
        }
        else {
            ERROR_A(c, s, d, "read urandom");
        }
        return;
    }
    d.size = size;
    RETURN_A(c, s, d);
}
