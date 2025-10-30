#ifndef LOGGER_H
#define LOGGER_H

extern void logger_errors_write(const char* fmt, ...);
extern void logger_requests_write(const char* fmt, ...);
extern void logger_changes_write(const char* fmt, ...);

extern int logger_changes_sumtime(char* buffer, int buffer_size, const char* pattern);

#endif // LOGGER_H
