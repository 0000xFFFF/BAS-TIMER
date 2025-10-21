#ifndef LOGGER_H
#define LOGGER_H

void logger_errors_write(const char* fmt, ...);
void logger_requests_write(const char* fmt, ...);
void logger_changes_write(const char* fmt, ...);

int logger_sumtime(char* buffer, int buffer_size, const char* filename, const char* pattern);

#endif // LOGGER_H
