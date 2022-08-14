#include "meloop/logging.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


int logging_fd = STDERR_FILENO;
enum logging_verbosity logging_verbosity = LOGGING_DEBUG;


const char * logging_verbosities[] = {
    "silent",  // 0
    "errr",    // 1
    "warn",    // 2
    "info",    // 3
    "dbug",    // 4
};


void
logging_log(
        enum logging_verbosity level, 
        const char *filename,
        int lineno,
        const char *function,
        const char *format, 
        ...) {
   
    va_list args;

    if (level > logging_verbosity) {
        return;
    }
    
    dprintf(logging_fd, "[%-4s] [%s:%d %s]",
        logging_verbosities[level],
        filename,
        lineno,
        function
    );
    
    if (format) { 
        va_start(args, format);
        dprintf(logging_fd, " ");
        vdprintf(logging_fd, format, args);
        va_end(args);
    }

    if (errno && (level != LOGGING_INFO)) {
        dprintf(logging_fd, " -- %s. errno: %d", strerror(errno), errno);
    }

    dprintf(logging_fd, CR);
    fsync(logging_fd);
}


enum logging_verbosity 
logging_verbosity_from_string(char * verbosity) {
    switch (verbosity[0]) {
        case 's':
            return LOGGING_SILENT;
        case 'e': 
            return LOGGING_ERROR;
        case 'w': 
            return LOGGING_WARNING; 
        case 'i': 
            return LOGGING_INFO;
        case 'd': 
        default:
            return LOGGING_DEBUG;
    }
}
