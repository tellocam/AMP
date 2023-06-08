#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>
#include <threads.h>


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


//define number of threads
#define N 8

typedef struct {
    bool flags[N];
    int tail;
} Array_lock_t;


void lock_init(Array_lock_t* lock) {
    lock->tail = 0;
    lock->flags[0] = true;
    for (int i=1; i < N; i++){
        lock->flags[i] = false;  
    }
}

void lock_acquire(Array_lock_t* lock, int *mySlot) {
    *mySlot = atomic_fetch_add(&lock->tail,1)%N;
    while (!lock->flags[*mySlot]) {};
}

void lock_release(Array_lock_t* lock, int *mySlot) {
    //#pragma omp atomic
    lock->flags[*mySlot] = false;
    lock->flags[(*mySlot + 1) % N] = true;
}


int main() {
    Array_lock_t lock;
    lock_init(&lock); // Initialize lock

    // Number of threads launched -> will be read from cmd line later
    int n = N; //N is defined globally above

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
        //int* shared_served = &served;
        while (count_total < 20-n) 
        {
            // critical_section(count_success, count_total);

            // Acquire lock
            lock_acquire(&lock, );

            // Critical section
            // sleepForOneCycle();
            int tid = omp_get_thread_num();
            count_success[tid] += 1;
            printf("Thread %d has acquired %d times.\n", tid, count_success[tid]);
            count_total += 1;

            // Release lock
            lock_release(&lock);
        }
    }

    for (int i = 0; i < n; i++)
    {
        printf("Thread %d: %d / %d\n", i, count_success[i], count_total+1);
    }
    
    return 0;
}