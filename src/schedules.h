#ifndef SCHEDULES_H
#define SCHEDULES_H

// Linked list node definition
#include "request.h"

extern pthread_mutex_t g_mutex_schedules;

struct HeatScheduleNode {
    int id;
    struct HeatSchedule data;
    struct HeatScheduleNode* next; // Pointer to next node
};

extern struct HeatScheduleNode* g_schedules;
extern void schedules_init(void);
extern void schedules_create(int from, int to, uint64_t duration);
extern void schedules_delete(int id);
extern void schedules_free(void);

#endif // SCHEDULES_H
