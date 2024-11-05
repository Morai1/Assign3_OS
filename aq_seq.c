/**
 * @file   aq_seq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Sequential alarm queue implementation
 */

#include "aq.h"
#include <stdlib.h>

typedef struct {
    void **normal;  // Array for normal messages
    void *alarm;    // Pointer for a single alarm message
    int count;      // Number of normal messages
} AlarmQueueStruct;

// Only keep this implementation of `aq_create()`
AlarmQueue aq_create() {
    AlarmQueueStruct *q = (AlarmQueueStruct *)malloc(sizeof(AlarmQueueStruct));
    if (!q) return NULL;  // Memory allocation failed

    q->normal = (void **)malloc(sizeof(void *) * 100);  // Adjust size as needed
    if (!q->normal) {
        free(q);
        return NULL;  // Memory allocation failed
    }
    q->alarm = NULL;
    q->count = 0;
    return (AlarmQueue)q;
}

int aq_send(AlarmQueue q, void *msg, MsgKind kind) {
    if (!msg) return AQ_NULL_MSG;  // Ensure msg is not NULL

    AlarmQueueStruct *queue = (AlarmQueueStruct *)q;
    if (kind == AQ_ALARM) {
        if (queue->alarm) {
            return AQ_NO_ROOM;  // Alarm slot already occupied
        }
        queue->alarm = msg;
    } else {
        if (queue->count >= 100) {  // Adjust size as needed
            return AQ_NO_ROOM;  // No room for more normal messages
        }
        queue->normal[queue->count++] = msg;
    }
    return AQ_OK;
}

int aq_recv(AlarmQueue q, void **pmsg) {
    AlarmQueueStruct *queue = (AlarmQueueStruct *)q;
    if (queue->alarm) {
        *pmsg = queue->alarm;
        queue->alarm = NULL;  // Clear the alarm slot
        return AQ_ALARM;
    } else if (queue->count > 0) {
        *pmsg = queue->normal[0];
        for (int i = 1; i < queue->count; i++) {
            queue->normal[i - 1] = queue->normal[i];  // Shift messages left
        }
        queue->count--;
        return AQ_NORMAL;
    } else {
        return AQ_NO_MSG;  // No messages available
    }
}

int aq_size(AlarmQueue q) {
    AlarmQueueStruct *queue = (AlarmQueueStruct *)q;
    return queue->count + (queue->alarm ? 1 : 0);
}

int aq_alarms(AlarmQueue q) {
    AlarmQueueStruct *queue = (AlarmQueueStruct *)q;
    return (queue->alarm != NULL) ? 1 : 0;
}