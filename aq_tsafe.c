/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include "aq.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct MessageNode {
    void* message;
    struct MessageNode* next;
} MessageNode;

typedef struct AlarmQueue {
    MessageNode* normal_queue;    // Queue to store normal messages
    void* alarm_message;          // Single alarm message
    int has_alarm;                // Flag indicating if there's an alarm in the queue
    int normal_count;             // Counter for normal messages

    pthread_mutex_t lock;         // Mutex for synchronizing access to the queue
    pthread_cond_t cond_nonempty; // Condition variable for checking non-empty queue
    pthread_cond_t cond_alarm;    // Condition variable for alarm message handling
} AlarmQueue_t;

AlarmQueue aq_create() {
    AlarmQueue_t* queue = malloc(sizeof(AlarmQueue_t));
    if (!queue) return NULL;

    // Initialize fields
    queue->normal_queue = NULL;
    queue->alarm_message = NULL;
    queue->has_alarm = 0;
    queue->normal_count = 0;

    // Initialize mutex and condition variables
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond_nonempty, NULL);
    pthread_cond_init(&queue->cond_alarm, NULL);

    return (AlarmQueue)queue;
}

int aq_send(AlarmQueue aq, void* msg, MsgKind kind) {
    AlarmQueue_t* queue = (AlarmQueue_t*)aq;
    pthread_mutex_lock(&queue->lock);

    if (kind == AQ_ALARM) {
        // Block if an alarm message is already in the queue
        while (queue->has_alarm) {
            pthread_cond_wait(&queue->cond_alarm, &queue->lock);
        }
        queue->alarm_message = msg;
        queue->has_alarm = 1;
    }
    else {
        // Add a normal message to the normal queue
        MessageNode* new_node = malloc(sizeof(MessageNode));
        new_node->message = msg;
        new_node->next = NULL;

        if (queue->normal_queue == NULL) {
            queue->normal_queue = new_node;
        }
        else {
            MessageNode* current = queue->normal_queue;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = new_node;
        }
        queue->normal_count++;
    }

    // Signal to indicate a new message is available
    pthread_cond_signal(&queue->cond_nonempty);
    pthread_mutex_unlock(&queue->lock);
    return 0;
}

int aq_recv(AlarmQueue aq, void** pmsg) {
    AlarmQueue_t* queue = (AlarmQueue_t*)aq;
    pthread_mutex_lock(&queue->lock);

    // Wait until there is a message (either alarm or normal)
    while (!queue->has_alarm && queue->normal_count == 0) {
        pthread_cond_wait(&queue->cond_nonempty, &queue->lock);
    }

    if (queue->has_alarm) {
        // Retrieve the alarm message
        *pmsg = queue->alarm_message;
        queue->alarm_message = NULL;
        queue->has_alarm = 0;

        // Signal cond_alarm to unblock any waiting alarm sends
        pthread_cond_signal(&queue->cond_alarm);
        pthread_mutex_unlock(&queue->lock);
        return AQ_ALARM;
    }
    else {
        // Retrieve a normal message if no alarm is present
        MessageNode* node = queue->normal_queue;
        *pmsg = node->message;
        queue->normal_queue = node->next;
        free(node);
        queue->normal_count--;

        pthread_mutex_unlock(&queue->lock);
        return AQ_NORMAL;
    }
}

int aq_size(AlarmQueue aq) {
    AlarmQueue_t* queue = (AlarmQueue_t*)aq;
    pthread_mutex_lock(&queue->lock);
    int size = queue->normal_count + (queue->has_alarm ? 1 : 0);
    pthread_mutex_unlock(&queue->lock);
    return size;
}

int aq_alarms(AlarmQueue aq) {
    AlarmQueue_t* queue = (AlarmQueue_t*)aq;
    pthread_mutex_lock(&queue->lock);
    int alarms = queue->has_alarm;
    pthread_mutex_unlock(&queue->lock);
    return alarms;
}

void aq_destroy(AlarmQueue aq) {
    AlarmQueue_t* queue = (AlarmQueue_t*)aq;

    // Destroy the mutex and condition variables
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->cond_nonempty);
    pthread_cond_destroy(&queue->cond_alarm);

    // Free all remaining normal messages
    MessageNode* current = queue->normal_queue;
    while (current != NULL) {
        MessageNode* next = current->next;
        free(current);
        current = next;
    }

    free(queue);
}


