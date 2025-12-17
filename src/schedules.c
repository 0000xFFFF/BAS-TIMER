#include "schedules.h"
#include "globals.h"
#include "request.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

// Global linked list of schedules
struct Node* gl_schedules = NULL;

// Create a new node
struct Node* createNode(struct HeatSchedule value)
{
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (!newNode) {
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    newNode->data = value;
    newNode->next = NULL;
    return newNode;
}

// Insert at beginning of the list
void insertAtBeginning(struct Node** head, struct HeatSchedule value)
{
    struct Node* newNode = createNode(value);
    newNode->next = *head;
    *head = newNode;
}

// Insert at end of the list
void insertAtEnd(struct Node** head, struct HeatSchedule value)
{
    struct Node* newNode = createNode(value);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    struct Node* temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

// Compare two schedules (fixed: removed valid, now compares actual data fields)
int schedulesEqual(struct HeatSchedule a, struct HeatSchedule b)
{
    return (a.from == b.from) &&
           (a.to == b.to) &&
           (a.duration == b.duration) &&
           (a.last_yday == b.last_yday);
}

// Delete a schedule from the list
void deleteNode(struct Node** head, struct HeatSchedule key)
{
    struct Node* temp = *head;
    struct Node* prev = NULL;

    while (temp != NULL && !schedulesEqual(temp->data, key)) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    if (prev == NULL) { // node is head
        *head = temp->next;
    }
    else {
        prev->next = temp->next;
    }

    free(temp);
}

// Save schedules as text file
void saveSchedulesText(struct Node* head, const char* filename)
{
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing text");
        return;
    }

    struct Node* temp = head;
    while (temp != NULL) {
        // Removed valid field
        fprintf(file, "%d %d %d %d\n",
                temp->data.from,
                temp->data.to,
                temp->data.duration,
                temp->data.last_yday);
        temp = temp->next;
    }

    fclose(file);
}

// Load schedules from text file
struct Node* loadSchedulesText(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        // Not an error, just means no file exists yet
        return NULL;
    }

    struct Node* head = NULL;
    struct HeatSchedule hs;

    while (fscanf(file, "%d %d %d %d",
                  &hs.from,
                  &hs.to,
                  &hs.duration,
                  &hs.last_yday) == 4) {
        insertAtEnd(&head, hs);
    }

    fclose(file);
    return head;
}

// Save schedules as binary file
void saveSchedulesBinary(struct Node* head, const char* filename)
{
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing binary");
        return;
    }

    struct Node* temp = head;
    while (temp != NULL) {
        fwrite(&temp->data, sizeof(struct HeatSchedule), 1, file);
        temp = temp->next;
    }

    fclose(file);
}

// Load schedules from binary file
struct Node* loadSchedulesBinary(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL; // File may not exist yet
    }

    struct Node* head = NULL;
    struct HeatSchedule hs;

    while (fread(&hs, sizeof(struct HeatSchedule), 1, file) == 1) {
        insertAtEnd(&head, hs);
    }

    fclose(file);
    return head;
}


// Free all nodes in linked list
void freeSchedules(struct Node* head)
{
    struct Node* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}







// Create a HeatSchedule
struct HeatSchedule create_heat_schedule(int from, int to, int dur)
{
    struct HeatSchedule schedule;
    schedule.from = from;
    schedule.to = to;
    schedule.duration = dur;
    schedule.last_yday = -1;
    return schedule;
}

// Quick schedule creation with default +15min window, 5min duration
static struct HeatSchedule create_default(int from)
{
    int to = from + hms_to_today_seconds(0, 15, 0); // +15 min
    int dur = hms_to_today_seconds(0, 5, 0);        // 5 min
    return create_heat_schedule(from, to, dur);
}

// Initialize schedules: try to load binary, else create defaults and save both binary & text
void schedules_init()
{
    gl_schedules = loadSchedulesBinary(VAR_DIR_FILE_SCHEDULES_BIN);
    // gl_schedules = loadSchedulesText(VAR_DIR_FILE_SCHEDULES_TXT);

    if (gl_schedules == NULL) { // No file, create default schedules
        insertAtEnd(&gl_schedules, create_default(hms_to_today_seconds(1, 0, 0)));
        insertAtEnd(&gl_schedules, create_default(hms_to_today_seconds(5, 0, 0)));
        insertAtEnd(&gl_schedules, create_default(hms_to_today_seconds(9, 0, 0)));

        saveSchedulesBinary(gl_schedules, VAR_DIR_FILE_SCHEDULES_BIN);
        // saveSchedulesText(gl_schedules, VAR_DIR_FILE_SCHEDULES_TXT); // TEXT file is separate
    }
}

void schedules_create(int from, int to, int duration)
{
    insertAtEnd(&gl_schedules, create_heat_schedule(from, to, duration));
}

// Free all nodes in linked list
void schedules_free()
{
    freeSchedules(gl_schedules);
}
