#ifndef SCHEDULES_H
#define SCHEDULES_H

// Linked list node definition
#include "request.h"

struct HeatScheduleNode {
    struct HeatSchedule data;
    struct HeatScheduleNode* next; // Pointer to next node
};

extern struct HeatScheduleNode* gl_schedules;
extern void schedules_init(void);
extern void schedules_create(int from, int to, uint64_t duration);
extern void schedules_delete(int from, int to, uint64_t duration);
extern void schedules_free(void);

#endif // SCHEDULES_H
