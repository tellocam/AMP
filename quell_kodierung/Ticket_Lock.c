#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

#include <unistd.h> // für sleep()

/* These structs should match the definition in the python files.. this is to be done yet in a less cumbersome way
 */

struct counters {
    int failed_lockAcq;
    int successful_lockAcq;
};

struct bench_result {
    float time;
    struct counters reduced_counters;
};

void sleepForOneCycle() {
    __asm__ volatile("nop");
}

// Test-and-Test-and-Set Lock struct
typedef struct {
    int ticket;
    // int served;
} TATAS_lock_t;

// Initiate a "False" flagged Lock, this means, the lock is NOT acquired by any thread!
void lock_init(TATAS_lock_t* lock) {
    lock->ticket = 0;
}

void lock_acquire(TATAS_lock_t* lock, int *served) {
    int next = atomic_fetch_add_explicit(&lock->ticket,1, 0);
    while (*served < next) {};
}

void lock_release(int* served) {
    #pragma omp atomic
    (*served)++;
}

TATAS_lock_t lock; // Declare a test-and-set lock

// void critical_section() {
//     // Acquire lock
//     int served = 0;
//     lock_acquire(&lock, &served);

//     // Critical section
//     // sleepForOneCycle();
//     printf("Thread %d is in the critical section.\n", omp_get_thread_num());

//     // Release lock
//     lock_release(&served);
// }

// int main() {
//     TATAS_lock_init(&lock); // Initialize the test-and-set lock

//     // Set the number of threads
//     omp_set_num_threads(20);

//     // Parallel region
//     #pragma omp parallel
//     {
//         critical_section();
//     }

//     return 0;
// }

int main() {
    lock_init(&lock); // Initialize the test-and-set lock

    // Number of threads launched -> will be read from cmd line later
    int n = 8;

    // Create counters
    int count_success[n]; 
    for (int i = 0; i < n; i++)
    {
        count_success[i] = 0;
    }
 
    int count_total = 0;
    int served = 0;

    // Set the number of threads
    omp_set_num_threads(n);

    // Parallel region
    #pragma omp parallel
    {
        int* shared_served = &served;
        while (count_total < 1000-n) 
        {
            // critical_section(count_success, count_total);

            // Acquire lock
            lock_acquire(&lock, shared_served);

            // Critical section
            // sleepForOneCycle();
            int tid = omp_get_thread_num();
            count_success[tid] += 1;
            // printf("Thread %d is has acquired %d times with ticket %d.\n", tid, count_success[tid], served);
            count_total += 1;

            // Release lock
            lock_release(shared_served);
        }
    }

    for (int i = 0; i < n; i++)
    {
        printf("Thread %d: %d / %d\n", i, count_success[i], count_total+1);
    }
    
    return 0;
}