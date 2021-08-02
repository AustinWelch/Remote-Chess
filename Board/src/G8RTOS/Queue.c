#include "Queue.h"
#include "G8RTOS_Scheduler.h"

static Queue queues[MAX_QUEUES];

QueueResult Queue_Create(void) {
    static uint8_t numQueues = 0;

    if (numQueues == MAX_QUEUES)
        return (QueueResult) { .queue = NULL, .status = QUEUE_MAX_QUEUES };

    Queue* queue = &queues[numQueues++];

    queue->head = 0;
    queue->tail = 0;

    queue->accessSem = (Semaphore) { 1 };
    queue->curSizeSem = (Semaphore) { 0 };
    
    queue->dataLost = 0;

    return (QueueResult) { .queue = queue, QUEUE_SUCCESS };
}

// Unthreaded size function that does not attempt to lock mutex
static uint16_t detail_Queue_GetSize(const Queue* queue) {
    return queue->curSizeSem.count;
}

QueueStatus TQueue_Push(Queue* queue, int_fast32_t value) {
    const bool_t inPThread = G8RTOS_InPThread();

    // Check if in pthread. If not, get exclusive control. Else, check if the pthread can take without blocking.
    if (!inPThread)
        G8RTOS_AcquireSemaphore(&queue->accessSem);
    else if (!G8RTOS_IsSemaphoreAvailable(&queue->accessSem)) {
        queue->dataLost++;

        return QUEUE_CANNOT_BLOCK_PTHREAD;
    }
    
    // Check if full
    if (detail_Queue_GetSize(queue) == QUEUE_SIZE) {
        queue->dataLost++;

        // We would only have the semaphore if we aren't in a pthread, so check
        if (!inPThread)
            G8RTOS_ReleaseSemaphore(&queue->accessSem);

        return QUEUE_FULL;
    }

    // Add the data then increment head circularly
    queue->data[queue->head] = value;
    queue->head++;
    
    if (queue->head == QUEUE_SIZE)
        queue->head = 0;


    // Increment the current size by releasing this semaphore, allowing blocked accesses to read
    // This is OK to do in a pthread.
    G8RTOS_ReleaseSemaphore(&queue->curSizeSem); 

    if (!inPThread)
        G8RTOS_ReleaseSemaphore(&queue->accessSem);

    return QUEUE_SUCCESS;
}

int_fast32_t TQueue_Pop(Queue* queue) {
    const bool_t inPThread = G8RTOS_InPThread();

    // Check if in pthread. If not, get exclusive control. If we are, we need to check if anyone else is using
    if (!inPThread) {
        G8RTOS_AcquireSemaphore(&queue->curSizeSem); // Wait for data to arrive. We do not release this semaphore.
        G8RTOS_AcquireSemaphore(&queue->accessSem); // Wait for exclusive control
    } else if (detail_Queue_GetSize(queue) == 0 || !G8RTOS_IsSemaphoreAvailable(&queue->accessSem))
        return 0;


    int_fast32_t value = queue->data[queue->tail];

    queue->tail++;

    if (queue->tail == QUEUE_SIZE)
        queue->tail = 0;

    if (!inPThread)
        G8RTOS_ReleaseSemaphore(&queue->accessSem);

    return value;
}

uint16_t TQueue_GetSize(Queue* queue) {
    if (!G8RTOS_InPThread()) {
        G8RTOS_AcquireSemaphore(&queue->accessSem);
        uint16_t s = detail_Queue_GetSize(queue);
        G8RTOS_ReleaseSemaphore(&queue->accessSem);

        return s;
    } else {
        return detail_Queue_GetSize(queue);
    }
}

bool_t TQueue_IsFull(Queue* queue) {
    return TQueue_GetSize(queue) == QUEUE_SIZE;
}

QueueStatus Queue_Push(Queue* queue, int_fast32_t value) {
    const bool_t inPThread = G8RTOS_InPThread();

    // Check if full
    if (detail_Queue_GetSize(queue) == QUEUE_SIZE) {
        queue->dataLost++;

        return QUEUE_FULL;
    }

    // Add the data then increment head circularly
    queue->data[queue->head] = value;
    queue->head++;
    
    if (queue->head == QUEUE_SIZE)
        queue->head = 0;

    queue->curSizeSem.count++;

    return QUEUE_SUCCESS;
}

int_fast32_t Queue_Pop(Queue* queue) {
    if (detail_Queue_GetSize(queue) == 0)
        return 0;

    int_fast32_t value = queue->data[queue->tail];

    queue->tail++;

    if (queue->tail == QUEUE_SIZE)
        queue->tail = 0;

    queue->curSizeSem.count--;

    return value;
}

uint16_t Queue_GetSize(Queue* queue) {
    return (uint16_t) queue->curSizeSem.count;
}

bool_t Queue_IsFull(Queue* queue) {
    return queue->curSizeSem.count == QUEUE_SIZE;
}
