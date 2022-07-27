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
//             waitA(c, conn, p, EPOLLIN);
//         }
//         else {
//             errorA(c, conn, "write");
//         }
//         return;
//     }
//     returnA(c, conn, NULL);
// }
