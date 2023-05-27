#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

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
    int flag;
} TATAS_lock_t;

// Initiate a "False" flagged Lock, this means, the lock is NOT acquired by any thread!
void TATAS_lock_init(TATAS_lock_t* lock) {
    lock->flag = 0;
}

void TATAS_lock_acquire(TATAS_lock_t* lock) {
    do {
        while (lock->flag == 1);
    } while (atomic_flag_test_and_set(&lock->flag));
}

void TATAS_lock_release(TATAS_lock_t* lock) {
    atomic_flag_clear(&lock->flag);
}

TATAS_lock_t lock; // Declare a test-and-set lock

void critical_section() {
    // Acquire lock
    TATAS_lock_acquire(&lock);

    // Critical section
    // sleepForOneCycle();
    printf("Thread %d is in the critical section.\n", omp_get_thread_num());

    // Release lock
    TATAS_lock_release(&lock);
}

int main() {
    TATAS_lock_init(&lock); // Initialize the test-and-set lock

    // Set the number of threads
    omp_set_num_threads(4);

    // Parallel region
    #pragma omp parallel
    {
        critical_section();
    }

    return 0;
}