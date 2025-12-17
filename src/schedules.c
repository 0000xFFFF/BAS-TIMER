#include "schedules.h"
#include "globals.h"
#include "request.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

pthread_mutex_t g_mutex_schedules = PTHREAD_MUTEX_INITIALIZER;

static int file_exists(const char* path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

// Global linked list of schedules
struct HeatScheduleNode* g_schedules = NULL;

// Create a new node
static struct HeatScheduleNode* createNode(struct HeatSchedule value, int id)
{
    struct HeatScheduleNode* newNode = (struct HeatScheduleNode*)malloc(sizeof(struct HeatScheduleNode));
    if (!newNode) {
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    newNode->data = value;
    newNode->next = NULL;
    newNode->id = id;
    return newNode;
}

// Insert at end (helper)
static void insertAtEnd(struct HeatScheduleNode** head, struct HeatSchedule value)
{
    struct HeatScheduleNode* temp = *head;

    // Check for duplicate
    while (temp) {
        if (temp->data.from == value.from &&
            temp->data.to == value.to &&
            temp->data.duration == value.duration) {
            return; // Duplicate, do not insert
        }
        temp = temp->next;
    }

    // Compute new index
    int new_index = 0;
    temp = *head;
    while (temp && temp->next != NULL) {
        temp = temp->next;
        new_index = temp->id + 1;
    }

    struct HeatScheduleNode* newNode = createNode(value, new_index);

    if (*head == NULL) {
        *head = newNode;
        return;
    }

    temp->next = newNode;
}

// Delete node (helper)
static void deleteNode(struct HeatScheduleNode** head, int id)
{
    struct HeatScheduleNode* temp = *head;
    struct HeatScheduleNode* prev = NULL;

    while (temp != NULL && temp->id != id) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    if (prev == NULL) { // head
        *head = temp->next;
    }
    else {
        prev->next = temp->next;
    }

    free(temp);
}

// Save/load binary
static void saveSchedulesBinary(struct HeatScheduleNode* head, const char* filename)
{
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing binary");
        return;
    }

    struct HeatScheduleNode* temp = head;
    while (temp != NULL) {
        fwrite(&temp->data, sizeof(struct HeatSchedule), 1, file);
        temp = temp->next;
    }
    fclose(file);
}

static struct HeatScheduleNode* loadSchedulesBinary(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;

    struct HeatScheduleNode* head = NULL;
    struct HeatSchedule hs = {0};

    while (fread(&hs, sizeof(struct HeatSchedule), 1, file) == 1) {
        insertAtEnd(&head, hs);
    }

    fclose(file);
    return head;
}

// Free list
static void freeSchedules(struct HeatScheduleNode* head)
{
    struct HeatScheduleNode* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

// Create a schedule
static struct HeatSchedule create_heat_schedule(int from, int to, uint64_t dur)
{
    struct HeatSchedule s;
    s.from = from;
    s.to = to;
    s.duration = dur;
    s.last_yday = -1;
    return s;
}

// Quick default schedule
static struct HeatSchedule create_default(int from)
{
    int to = from + hms_to_today_seconds(0, 15, 0);
    uint64_t dur = (uint64_t)hms_to_today_seconds(0, 5, 0);
    return create_heat_schedule(from, to, dur);
}

// Initialize schedules
void schedules_init(void)
{
    pthread_mutex_lock(&g_mutex_schedules);

    if (file_exists(VAR_DIR_FILE_SCHEDULES_BIN)) {
        g_schedules = loadSchedulesBinary(VAR_DIR_FILE_SCHEDULES_BIN);
    }
    else {
        insertAtEnd(&g_schedules, create_default(hms_to_today_seconds(1, 0, 0)));
        insertAtEnd(&g_schedules, create_default(hms_to_today_seconds(5, 0, 0)));
        insertAtEnd(&g_schedules, create_default(hms_to_today_seconds(9, 0, 0)));

        saveSchedulesBinary(g_schedules, VAR_DIR_FILE_SCHEDULES_BIN);
    }

    pthread_mutex_unlock(&g_mutex_schedules);
}

// Create a schedule (prevents duplicates)
void schedules_create(int from, int to, uint64_t duration)
{
    pthread_mutex_lock(&g_mutex_schedules);
    insertAtEnd(&g_schedules, create_heat_schedule(from, to, duration));
    saveSchedulesBinary(g_schedules, VAR_DIR_FILE_SCHEDULES_BIN);
    pthread_mutex_unlock(&g_mutex_schedules);
}

// Delete a schedule
void schedules_delete(int id)
{
    pthread_mutex_lock(&g_mutex_schedules);
    deleteNode(&g_schedules, id);
    saveSchedulesBinary(g_schedules, VAR_DIR_FILE_SCHEDULES_BIN);
    pthread_mutex_unlock(&g_mutex_schedules);
}

// Free all schedules
void schedules_free(void)
{
    pthread_mutex_lock(&g_mutex_schedules);
    freeSchedules(g_schedules);
    g_schedules = NULL;
    pthread_mutex_unlock(&g_mutex_schedules);
}
