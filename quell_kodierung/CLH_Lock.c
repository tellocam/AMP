#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdatomic.h>
#include <stdbool.h>

// Node structure for the CLH queue
typedef struct clh_node {
    atomic_bool locked;
    struct clh_node* prev;
} clh_node;

// CLH queue structure
typedef struct clh_lock {
    clh_node* tail;
    clh_node node;
} clh_lock;

// Initialize the CLH lock
void lock_init(clh_lock* lock) {
    lock->tail = NULL;
    atomic_init(&lock->node.locked, false);
    lock->node.prev = NULL;
}

// Acquire the CLH lock
void lock_acquire(clh_lock* lock) {
    clh_node* node = &lock->node;
    atomic_store_explicit(&node->locked, true, memory_order_release);

    clh_node* pred = atomic_exchange_explicit(&lock->tail, node, memory_order_acq_rel);
    if (pred != NULL) {
        node->prev = pred;
        while (atomic_load_explicit(&pred->locked, memory_order_acquire)) {
            // Spin-wait
        }
        node->prev = NULL;
    }
}

// Release the CLH lock
void lock_release(clh_lock* lock) {
    clh_node* node = &lock->node;
    atomic_store_explicit(&node->locked, false, memory_order_release);
    if (node->prev == NULL) {
        clh_node* succ = node;
        if (!atomic_compare_exchange_weak_explicit(&lock->tail, &succ, NULL, memory_order_release, memory_order_relaxed)) {
            while (node->prev == NULL) {
                // Spin-wait 
            }
        }
    }
}

// int main() {
//     // Number of threads launched -> to be read from cmd line later
//     const int num_threads = 8;

//     // Create + initialize the lock
//     clh_lock lock;
//     lock_init(&lock);

//     // Shared variable protected by the lock
//     int counter = 0;

//     // Parallel region with multiple threads
//     #pragma omp parallel num_threads(num_threads) shared(lock, counter)
//     {
//         // Acquire the lock
//         lock_acquire(&lock);

//         // Critical section
//         counter++;
//         printf("Thread %d incremented counter: %d\n", omp_get_thread_num(), counter);

//         // Release the lock
//         lock_release(&lock);
//     }

//     return 0;
// }

int main() {
    
    // Create + initialize the lock
    clh_lock lock;
    lock_init(&lock);
    
    // Number of threads launched -> to be read from cmd line later
    const int n = 8;

    // Create and prepare counters
    int count_success[n]; 
    for (int i = 0; i < n; i++) {count_success[i] = 0;}
    int count_total = 0;

    // Parallel region with multiple threads
    #pragma omp parallel num_threads(n) shared(lock, count_total, count_success)
    {
        while (count_total < 20-n) 
        {
            int tid = omp_get_thread_num();

            // Acquire lock
            lock_acquire(&lock);
            printf("Lock ACQUIRED by thread %d.\n", tid);

            // Critical section
            // sleepForOneCycle();
            // sleep(1);
            count_success[tid] += 1;
            printf("Thread %d is has acquired %d times.\n", tid, count_success[tid]);
            count_total += 1;

            // Release lock
            printf("Lock RELEASE coming up by thread %d.\n", tid);
            lock_release(&lock);
            printf("Lock RELEASED by thread %d.\n", tid);
        }
    }

return 0;

}



