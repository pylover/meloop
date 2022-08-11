#ifndef MELOOP_LOGGING_H
#define MELOOP_LOGGING_H
 

enum logging_verbosity {
    LOGGING_SILENT,     // 0
    LOGGING_ERROR,      // 1
    LOGGING_WARNING,    // 2
    LOGGING_INFO,       // 3
    LOGGING_DEBUG,      // 4
};


extern int logging_fd;
extern enum logging_verbosity logging_verbosity;
extern const char * logging_verbosities [];


enum logging_verbosity logging_verbosity_from_string(char * verbosity);


void
logging_log(
        enum logging_verbosity level, 
        const char *filename,
        int lineno,
        const char *function,
        const char *format, 
        ...);


#define LOG(l, ...) \
    logging_log(l, __FILE__, __LINE__ , __FUNCTION__, __VA_ARGS__)


#define DEBUG(...)   LOG(LOGGING_DEBUG,   __VA_ARGS__)
#define INFO(...)    LOG(LOGGING_INFO,    __VA_ARGS__)
#define WARNING(...) LOG(LOGGING_WARNING, __VA_ARGS__)
#define ERROR(...)   LOG(LOGGING_ERROR,   __VA_ARGS__)
#define FATAL(...)   LOG(LOGGING_ERROR,   __VA_ARGS__); exit(EXIT_FAILURE)


#endif
