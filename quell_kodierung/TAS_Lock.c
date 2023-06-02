#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

/* These structs should match the definition in the python files.. this is to be done yet in a less cumbersome way
 */
/*
non-atomic correctness struct
*/
struct counters {
    int failed_lockAcq;
    int successful_lockAcq;
};


struct bench_result {
    float time;
    struct counters reduced_counters;
};


// Test-and-Set Lock struct
typedef struct {
    int flag;
} TAS_lock_t;

// Initiate a "False" flagged Lock, this means, the lock is NOT acquired by any thread!
void TAS_lock_init(TAS_lock_t* lock) {
    lock->flag = 0;
}

void TAS_lock_acquire(TAS_lock_t* lock) {
    while (atomic_flag_test_and_set(&lock->flag)) {
        // Stay in WHILE part until the busy thread sets lock->flag = 0
        // Here we might introduce the non-atomic faileq_lockAcq +=

    }
}

void TAS_lock_release(TAS_lock_t* lock) {
    atomic_flag_clear(&lock->flag);
}

TAS_lock_t lock; // Declare a test-and-set lock

void critical_section() {
    // Acquire lock
    TAS_lock_acquire(&lock);

    // Critical section
    printf("Thread %d is in the critical section.\n", omp_get_thread_num());
    int tid = omp_get_thread_num();

    // Release lock
    TAS_lock_release(&lock);
}

int main() {
    TAS_lock_init(&lock); // Initialize the test-and-set lock

    // Number of threads launched -> will be read from cmd line later
    int n = 8;

    // Create counters
    int count_success[n]; 
    for (int i = 0; i < n; i++)
    {
        count_success[i] = 0;
    }
 
    int count_total = 0;

    // Set the number of threads
    omp_set_num_threads(n);

    // Parallel region
    #pragma omp parallel
    {

        while (count_total < 1000-n) 
        {
            // critical_section(count_success, count_total);

            // Acquire lock
            TAS_lock_acquire(&lock);

            // Critical section
            // sleepForOneCycle();
            // printf("Thread %d is in the critical section.\n", omp_get_thread_num());
            int tid = omp_get_thread_num();
            count_success[tid] += 1;
            count_total += 1;

            // Release lock
            TAS_lock_release(&lock);
        }
    }

    for (int i = 0; i < n; i++)
    {
        printf("Thread %d: %d / %d\n", i, count_success[i], count_total+1);
    }
    
    return 0;
}