#ifndef THREAD_REQUEST_DOCTOR_H
#define THREAD_REQUEST_DOCTOR_H

#include <stdatomic.h>

extern atomic_bool g_online;
extern void* th_request_doctor(void* sig);

#endif // THREAD_REQUEST_DOCTOR_H
