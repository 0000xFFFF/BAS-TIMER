#ifndef SCHEDULES_H
#define SCHEDULES_H

// Linked list node definition
#include "request.h"

struct Node {
    struct HeatSchedule data;
    struct Node* next; // Pointer to next node
};

extern struct Node* gl_schedules;
void schedules_init();
void schedules_create(int from, int to, int duration);
void schedules_free();

#endif // SCHEDULES_H
