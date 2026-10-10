#ifndef LOG_H
#define LOG_H

#define LOG_COLOR_RESET "\033[0m"
#define LOG_COLOR_INFO  "\033[0;33m"
#define LOG_COLOR_ERROR "\033[0;31m"

#define LOG_INFO(msg)                                                           \
    do {                                                                        \
        fprintf(stdout, LOG_COLOR_INFO "[INFO] - " msg LOG_COLOR_RESET "\n");   \
    } while (0)

#define LOG_INFO_ARGS(msg, ...)                                                 \
    do {                                                                        \
        fprintf(stdout, LOG_COLOR_INFO "[INFO] - " msg LOG_COLOR_RESET "\n",    \
                ##__VA_ARGS__);                                                 \
    } while (0)

#define LOG_ERROR(msg)                                                          \
    do {                                                                        \
        fprintf(stderr, LOG_COLOR_ERROR "[ERROR] - " msg LOG_COLOR_RESET "\n"); \
    } while (0)

#define LOG_ERROR_ARGS(msg, ...)                                                \
    do {                                                                        \
        fprintf(stderr, LOG_COLOR_ERROR "[ERROR] - " msg LOG_COLOR_RESET "\n",  \
                ##__VA_ARGS__);                                                 \
    } while (0)

#endif // LOG_H
