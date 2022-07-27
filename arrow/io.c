// #include "arrow/arrow.h"
// #include "arrow/io.h"
// 
// 
// writeA(struct circuit *c, struct conn *conn, struct packet p) {
//     ssize_t size;
// 
//     size = write(conn->wfd, p.data, p.size);
//     if (size < 0) {
//         if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
//             io_againA(c, conn, p, EPOLLIN);
//         }
//         else {
//             io_failedA(c, conn, "write");
//         }
//         return;
//     }
//     returnA(c, conn, NULL);
// }
