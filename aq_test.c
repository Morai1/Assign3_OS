
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "aq.h"

#include "auxiliary.h"

/**
 * Concurrent program that sends and receives a few integer messages
 * in order to demonstrate the basic use of the thread-safe Alarm Queue Library
 *
 * By using sleeps we (try to) control the scheduling of the threads in
 * order to obtain specific execution scenarios.  But there is no guarantee.
 *
 */

static AlarmQueue q;

void* producer(void* arg) {
    msleep(500);
    put_alarm(q, 1);
    printf("Producer: Sent first alarm message.\n");

    msleep(500);
    printf("Producer: Attempting to send second alarm message (should block).\n");
    put_alarm(q, 2);
    printf("Producer: Sent second alarm message (after unblocking).\n");

    put_normal(q, 3);
    printf("Producer: Sent normal message with value 3.\n");

    msleep(500);

    put_normal(q, 4);
    printf("Producer: Sent normal message with value 4.\n");

    put_normal(q, 5);
    printf("Producer: Sent normal message with value 5.\n");

    return 0;
}

void* consumer(void* arg) {
    int msg;
    msg = get(q);
    printf("Consumer: Received message with value %d  (First Alarm)\n", msg);

    msg = get(q);
    printf("Consumer: Received normal message with value %d\n", msg);

    msg = get(q);
    printf("Consumer: Received normal message with value %d\n", msg);

    msg = get(q);
    printf("Consumer: Received normal message with value %d\n", msg);

    msg = get(q);
    printf("Consumer: Received message with value %d  (Second Alarm)\n", msg);

    return 0;
}

int main(int argc, char** argv) {
    q = aq_create();
    if (q == NULL) {
        printf("Alarm queue could not be created\n");
        exit(1);
    }

    pthread_t t1, t2;
    void* res1, * res2;

    printf("----------------\n");

    pthread_create(&t1, NULL, producer, NULL);
    pthread_create(&t2, NULL, consumer, NULL);

    pthread_join(t1, &res1);
    pthread_join(t2, &res2);

    printf("----------------\n");
    printf("Threads terminated with %ld, %ld\n", (uintptr_t)res1, (uintptr_t)res2);

    print_sizes(q);
    aq_destroy(q);

    return 0;
}

