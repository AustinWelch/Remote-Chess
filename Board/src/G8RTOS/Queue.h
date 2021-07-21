#ifndef QUEUE_H
#define QUEUE_H

#include "G8RTOS_Defs.h"
#include "G8RTOS_Semaphores.h"

#define QUEUE_SIZE 16
#define MAX_QUEUES 4

typedef struct detail_Queue {
    int_fast32_t data[QUEUE_SIZE];
    uint16_t head;
    uint16_t tail;
    Semaphore curSizeSem;
    Semaphore accessSem;
    uint16_t dataLost;
} Queue;

typedef enum {
     QUEUE_SUCCESS = 0
    , QUEUE_FULL
    , QUEUE_MAX_QUEUES
    , QUEUE_CANNOT_BLOCK_PTHREAD
} QueueStatus;

typedef struct detail_QueueResult {
    Queue* queue;
    QueueStatus status;
} QueueResult;

QueueResult Queue_Create(void); // Create a queue with thread-safe operations

// TQueue = Threaded Queue. Allows thread-safe operation in both background threads and pthreads.

// Push data to a threaded queue. If full, it will throw away the data. Blocks for exclusive access.
// If currently in a Periodic Thread, it will not block but instead return with failure.
QueueStatus TQueue_Push(Queue* queue, int_fast32_t value);

// Pop data from threaded queue. If empty, block thread until data arrives. Blocks for exclusive access.
// If currently in a Periodic Thread, it will not block but instead return with garbage data if not exclusive access or empty
int_fast32_t TQueue_Pop(Queue* queue);

// QueueStatus Queue_PushUnthreaded(Queue* queue, int_fast32_t value); // Push data to queue. If full, throw away. Does not attempt to gain exclusive access
// int_fast32_t Queue_PopUnthreaded(Queue* queue); // Pop data. If empty, returns garbage.

uint16_t TQueue_GetSize(Queue* queue); // Get the size of a threaded queue
bool_t TQueue_IsFull(Queue* queue); // Check if a threaded queue is full

QueueStatus Queue_Push(Queue* queue, int_fast32_t value);
int_fast32_t Queue_Pop(Queue* queue);

uint16_t Queue_GetSize(Queue* queue); 
bool_t Queue_IsFull(Queue* queue);

#endif
