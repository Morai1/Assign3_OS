#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "aq.h"

// Global variables for test coordination
AlarmQueue queue;
pthread_t sender1, sender2, receiver;
volatile int senders_done = 0;  // Add counter for active senders

void* alarm_sender1(void* arg) {
    printf("Sender1: Sending first alarm message\n");
    char* msg1 = strdup("Alarm1");
    int result = aq_send(queue, msg1, AQ_ALARM);
    printf("Sender1: First alarm sent, result: %d\n", result);
    __sync_fetch_and_add(&senders_done, 1);  // Increment counter atomically
    return NULL;
}

void* alarm_sender2(void* arg) {
    printf("Sender2: Attempting to send second alarm message\n");
    char* msg2 = strdup("Alarm2");
    
    // Send a normal message first
    char* normal1 = strdup("Normal1");
    aq_send(queue, normal1, AQ_NORMAL);
    printf("Sender2: Sent normal message before attempting alarm\n");
    
    int result = aq_send(queue, msg2, AQ_ALARM);
    printf("Sender2: Second alarm sent after unblocking, result: %d\n", result);
    
    // Send another normal message
    char* normal2 = strdup("Normal2");
    aq_send(queue, normal2, AQ_NORMAL);
    printf("Sender2: Sent normal message after alarm\n");
    
    __sync_fetch_and_add(&senders_done, 1);  // Increment counter atomically
    return NULL;
}


void* message_receiver(void* arg) {
    while (1) {
        void* msg;
        int kind = aq_recv(queue, &msg);

        // Exit if the message is a shutdown signal (use a special kind or a NULL message)
        if (msg == NULL) {
            printf("Receiver: Received shutdown signal. Exiting.\n");
            break;
        }

        printf("Receiver: Received message: %s, type: %d\n", (char*)msg, kind);
        free(msg);

        msleep(500);  // Small delay between messages
    }
    return NULL;
}

int main() {
    // Create queue
    queue = aq_create();
    if (!queue) {
        printf("Failed to create alarm queue\n");
        return 1;
    }

    // Create threads
    pthread_create(&sender1, NULL, alarm_sender1, NULL);
    pthread_create(&sender2, NULL, alarm_sender2, NULL);
    pthread_create(&receiver, NULL, message_receiver, NULL);

    // Wait for all sender threads to complete
    pthread_join(sender1, NULL);
    pthread_join(sender2, NULL);

    // Send a shutdown signal to the queue (a NULL message)
    aq_send(queue, NULL, AQ_NORMAL);

    // Wait for the receiver thread to complete
    pthread_join(receiver, NULL);

    // Cleanup
    aq_destroy(queue);

    printf("Test completed.\n");
    return 0;
}



