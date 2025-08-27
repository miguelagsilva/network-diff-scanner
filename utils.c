#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdarg.h>

#include "main.h"
#include "utils.h"

const char* log_level_to_str(LogLevel level) {
    switch (level) {
        case LOG_ERROR: return "ERROR";
        case LOG_WARN:  return "WARN";
        case LOG_INFO:  return "INFO";
        case LOG_DEBUG: return "DEBUG";
        default:        return "UNKNOWN";
    }
}

void log_message(LogLevel log_level, const char *format, ...) {
  if (log_level > CURRENT_LOG_LEVEL ) return;

  time_t t = time(NULL);
  struct tm tmvar;
  localtime_r(&t, &tmvar);

  char dateTime[100];
  sprintf(dateTime, "%d/%02d/%02d %02d:%02d:%02d",
          tmvar.tm_year + 1900,
          tmvar.tm_mon + 1,
          tmvar.tm_mday,
          tmvar.tm_hour,
          tmvar.tm_min,
          tmvar.tm_sec);

  // Print log header
  fprintf(stderr, "[%s] %-5s: ", dateTime, log_level_to_str(log_level));

  // Print user message
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");

  if (log_level == LOG_ERROR) exit(EXIT_FAILURE);
}

void log_message_no_nl(LogLevel log_level, const char *format, ...) {
  if (log_level > CURRENT_LOG_LEVEL) return;

  time_t t = time(NULL);
  struct tm tmvar;
  localtime_r(&t, &tmvar);

  char dateTime[100];
  sprintf(dateTime, "%d/%02d/%02d %02d:%02d:%02d",
          tmvar.tm_year + 1900,
          tmvar.tm_mon + 1,
          tmvar.tm_mday,
          tmvar.tm_hour,
          tmvar.tm_min,
          tmvar.tm_sec);

  // Print log header
  fprintf(stderr, "[%s] %-5s: ", dateTime, log_level_to_str(log_level));

  // Print user message
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  if (log_level == LOG_ERROR) exit(EXIT_FAILURE);
}
