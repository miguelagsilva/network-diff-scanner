#ifndef UTILS_H
#define UTILS_H

typedef enum {
    LOG_ERROR = 0,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
} LogLevel;

#define CURRENT_LOG_LEVEL LOG_INFO

const char* log_level_to_str(LogLevel level);
void error(char message[]);
void log_message(LogLevel log_level, const char *format, ...);
void log_message_no_nl(LogLevel log_level, const char *format, ...);

#endif // !UTILS_H
