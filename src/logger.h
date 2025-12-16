#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>

extern void logger_write_errors(const char* fmt, ...);
extern void logger_write_requests(const char* fmt, ...);
extern void logger_write_changes(const char* fmt, ...);
extern void logger_wttrin_write(const char* fmt, ...);
extern size_t logger_changes_sumtime(char* buffer, size_t buffer_size, const char* pattern);

#endif // LOGGER_H
