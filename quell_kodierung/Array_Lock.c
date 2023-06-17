#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>
#include <threads.h>

// define number of threads
// #define N 8
// define thread local variables
__thread int mySlot;

typedef struct Array_lock_t{
    bool* flags;
    _Atomic int tail;
} Array_lock_t;



void lock_init(Array_lock_t* lock) {
    // allocate memory for flags
    int N = omp_get_num_threads();
    lock->flags = malloc(N*sizeof(bool));
    lock->flags[0] = true;
    for (int i=1; i < N; i++){
        lock->flags[i] = false;  
    }
    lock->tail = 0;
}

void lock_acquire(Array_lock_t* lock) {
    int N = omp_get_num_threads();
    mySlot = atomic_fetch_add(&lock->tail,1)%N;
    while (!lock->flags[mySlot]) {};
}

void lock_release(Array_lock_t* lock) {
    int N = omp_get_num_threads();
    lock->flags[mySlot] = false;
    lock->flags[(mySlot + 1) % N] = true;
}

// void lock_acquire(Array_lock_t* lock, int* mySlot) {
//     int N = omp_get_num_threads();
//     *mySlot = atomic_fetch_add(&lock->tail,1)%N;
//     while (!lock->flags[*mySlot]) {};
// }

// void lock_release(Array_lock_t* lock, int* mySlot) {
//     int N = omp_get_num_threads();
//     lock->flags[*mySlot] = false;
//     lock->flags[(*mySlot + 1) % N] = true;
// }


int main() {
    struct Array_lock_t* lock;
    lock = (struct Array_lock_t*)malloc(sizeof(struct Array_lock_t));
    lock_init(lock); // Initialize lock
    // static int* mySlot;
    // #pragma omp threadprivate(mySlot)
    // mySlot = (int*)malloc(sizeof(int));

    // Number of threads launched -> will be read from cmd line later
    int n = 8; //N is defined globally above

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
        while (count_total < 100000000-n) 
        {
            // critical_section(count_success, count_total);

            // Acquire lock
            lock_acquire(lock);
            // lock_acquire(lock, mySlot);

            // Critical section
            // sleepForOneCycle();
            int tid = omp_get_thread_num();
            count_success[tid] += 1;
            // printf("Thread %d has acquired %d times.\n", tid, count_success[tid]);
            count_total += 1;

            // Release lock
            lock_release(lock);
            // lock_release(lock, mySlot);
        }
    }

    for (int i = 0; i < n; i++)
    {
        printf("Thread %d: %d / %d\n", i, count_success[i], count_total+1);
    }
    
    free(lock->flags);
    return 0;
}