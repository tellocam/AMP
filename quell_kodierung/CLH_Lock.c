// int main() {
    
//     // Create + initialize the lock
//     clh_lock lock;
//     lock_init(&lock);
    
//     // Number of threads launched -> to be read from cmd line later
//     const int n = 8;

//     // Create and prepare counters
//     int count_success[n]; 
//     for (int i = 0; i < n; i++) {count_success[i] = 0;}
//     int count_total = 0;

//     // Parallel region with multiple threads
//     #pragma omp parallel num_threads(n) shared(lock, count_total, count_success)
//     {
//         while (count_total < 10-n) 
//         {
//             int tid = omp_get_thread_num();

//             // Acquire lock
//             lock_acquire(&lock);
//             printf("Lock ACQUIRED by thread %d.\n", tid);

//             // Critical section
//             // sleepForOneCycle();
//             // sleep(1);
//             count_success[tid] += 1;
//             printf("Thread %d is has acquired %d times.\n", tid, count_success[tid]);
//             count_total += 1;

//             // Release lock
//             printf("Lock RELEASE coming up by thread %d.\n", tid);
//             lock_release(&lock);
//             printf("Lock RELEASED by thread %d.\n", tid);
//         }
//     }

// return 0;

// }


#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdatomic.h>
#include <stdbool.h>

typedef struct clh_node {
    atomic_bool locked;
    struct clh_node* next;
} clh_node;

// Global tail node
clh_node* tail;

// Thread-local node
#pragma omp threadprivate(tail)
clh_node* node;

void lock() {
    node = (clh_node*)malloc(sizeof(clh_node));
    node->locked = true;
    node->next = NULL;

    clh_node* prev = atomic_exchange(&tail, node);
    prev->next = node;

    while (atomic_load_explicit(&node->locked, memory_order_acquire)) {
        // Spin-wait
    }
}

void unlock() {
    clh_node* next = node->next;
    node->locked = false;
    free(node);
    node = next;
}

int main() {
    // Initialize the tail node
    tail = (clh_node*)malloc(sizeof(clh_node));
    tail->locked = false;
    tail->next = NULL;

    // Number of threads launched -> to be read from cmd line later
    const int n = 8;

    // Create and prepare counters
    int count_success[n];
    for (int i = 0; i < n; i++) {
        count_success[i] = 0;
    }
    int count_total = 0;

    // Parallel region with multiple threads
    #pragma omp parallel num_threads(n) shared(count_total, count_success)
    {
        // Thread-local node
        clh_node* node;

        while (count_total < 10 - n) {
            int tid = omp_get_thread_num();

            // Acquire lock
            lock();
            printf("Lock ACQUIRED by thread %d.\n", tid);

            // Critical section
            count_success[tid] += 1;
            printf("Thread %d has acquired the lock %d times.\n", tid, count_success[tid]);
            count_total += 1;

            // Release lock
            printf("Lock RELEASE coming up by thread %d.\n", tid);
            unlock();
            printf("Lock RELEASED by thread %d.\n", tid);
        }
    }

    // Clean up the tail node
    free(tail);

    return 0;
}




