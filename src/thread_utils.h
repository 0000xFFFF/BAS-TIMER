#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

extern void init_thread_data();
extern void stop_all_threads();
extern struct timespec make_timeout_ms(long ms);
extern void sleep_ms_interruptible(long ms);

#endif // THREAD_UTILS_H
