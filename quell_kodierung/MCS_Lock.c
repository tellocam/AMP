#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h> //
#include <stdbool.h>
#include <omp.h> //
#include <stdatomic.h>
#include <unistd.h> // für sleep()

#include <stddef.h>

void sleepForOneCycle() {
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    __asm__ volatile("nop");
}

struct mcs_node {
    struct mcs_node* next;
    bool locked;
};

struct mcs_queue {
    atomic_intptr_t head;
    struct mcs_node* qnode;
};



// void lock_init(struct mcs_queue* queue) {
//     queue->head = (atomic_intptr_t)NULL;
//     queue->qnode->next = NULL;
//     queue->qnode->locked = false;
// }

void lock_init(struct mcs_queue* queue) {
    queue->head = (atomic_intptr_t)NULL;

    // Allocate memory for queue->qnode
    queue->qnode = (struct mcs_node*)malloc(sizeof(struct mcs_node));
    if (queue->qnode == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for queue->qnode\n");
        exit(1); // or return an appropriate error code
    }

    queue->qnode->next = NULL;
    queue->qnode->locked = false;
}

void lock_acquire(struct mcs_queue* queue) {
    struct mcs_node* pred = (struct mcs_node*)atomic_exchange(&queue->head, (intptr_t)&queue->qnode);

    if (pred != NULL) {
        queue->qnode->locked = true;
        pred->next = queue->qnode;
        int tid = omp_get_thread_num();
        while (queue->qnode->locked) {}
    }
}

void lock_release(struct mcs_queue* queue) {
    struct mcs_node* self = queue->qnode;

    if (self->next == NULL) {
        // No other threads are waiting -> release the lock
        if (atomic_compare_exchange_strong(&queue->head, (intptr_t*)&self, 0)) {
            return;
        }

        // Another thread is waiting, spin-wait until it sets self->next
        while (self->next == NULL) {}
    }

    // Notify the next waiting thread that it can acquire the lock
    self->next->locked = false;
}

int main () {

    // Allocate memory for queue
    struct mcs_queue* queue = (struct mcs_queue*)malloc(sizeof(struct mcs_queue));
    if (queue == NULL) {
        fprintf(stderr, "Failed to allocate memory for queue\n");
        exit(1); // or return an appropriate error code
    }
    // Initialize the lock
    // struct mcs_queue* queue;
    lock_init(queue); 

    // Number of threads launched -> will be read from cmd line later
    int n = 8;

    // Create and prepare counters
    int count_success[n]; 
    for (int i = 0; i < n; i++) {count_success[i] = 0;}
    int count_total = 0;

    // Set the number of threads
    omp_set_num_threads(n);
    

    // Parallel region
    #pragma omp parallel
    {
        while (count_total < 42-n) 
        {
            int tid = omp_get_thread_num();

            // Acquire lock
            lock_acquire(queue);
            printf("Lock ACQUIRED by thread %d.\n", tid);

            // Critical section
            sleepForOneCycle();
            // sleep(1);
            count_success[tid] += 1;
            printf("Thread %d is has acquired %d times.\n", tid, count_success[tid]);
            count_total += 1;

            // Release lock
            printf("Lock RELEASE coming up by thread %d.\n", tid);
            lock_release(queue);
            printf("Lock RELEASED by thread %d.\n", tid);
        }
    }

    for (int i = 0; i < n; i++)
    {
        printf("Thread %d: %d / %d\n", i, count_success[i], count_total+1);
    }
    
    free(queue);
    return 0;
}