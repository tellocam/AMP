#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

#include "../include/benchUtils.h" // Benchmark datatypes and functions

// Ticket Lock struct
typedef struct {
    int ticket;
} ticket_lock_t;

// Initiate a "False" flagged Lock, this means, the lock is NOT acquired by any thread!
static void lock_init(ticket_lock_t* lock) {
    lock->ticket = 0;
}

static void lock_acquire(ticket_lock_t* lock, int *served) {
    int next = atomic_fetch_add_explicit(&lock->ticket,1, 0);
    while (*served < next) {
        // Thread within spin!
    };
}

static void lock_release(int* served) {
    #pragma omp atomic
    (*served)++;
}


static void threadBench(ticket_lock_t* lock, threadBenchData* threadData, int* successCheck, int times, int sleepCycles) {
    
    while (*successCheck < times) {
        lock_acquire(lock, threadData); // Acquire Lock
        (*successCheck)++; //critical Section
        lock_release(lock); // Release lock
        threadBedtime(sleepCycles); // Take a Nap
    }
    
}

benchData benchTAS(int threads, int times, int sleepCycles) {

    benchData result;
    initializeBenchData(&result);
    threadBenchData thread_data[threads];
    for(int d = 0; d<threads; d++) {
        initializeThreadBenchData(&(thread_data[d]));
    }
    int successCheck = 0;

    ticket_lock_t TAS_lock;
    lock_init(&TAS_lock);


    double tic, toc;
    tic = omp_get_wtime();

        #pragma omp parallel
        omp_set_num_threads(threads); 
        {
            #pragma omp parallel for
            for (int i=0; i<threads; i++) {
                // &thread_data[0] is pointer to first entry of thread_data array, later used with pointer arithmetic
                threadBench(&TAS_lock, &thread_data[0], &successCheck, times, sleepCycles); 
            }   
        }

    toc = omp_get_wtime();
    result.time = (toc - tic);

    for (int i=0; i<threads; i++) {
        result.success += thread_data[i].success; // total success
        result.fail     += thread_data[i].fail; // total fails
        result.wait += thread_data[i].wait/threads; // avg wait per thread
        result.fairness_dev += 100 * (abs(thread_data[i].success - times/threads) / (float)times); //avg fairness deviation in %
    }

    result.throughput = result.success / result.time;

    // printf("TAS Lock Summary: %d Lock acquisiton requests on %d threads took: %f\n",
    //        times, threads, result.time);
    // printf("  with %d failAcq,  %d successAcq, %f %% fairness dev.,  %f  acq/s throughput\n",
    //     result.fail,
    //     result.success,
    //     result.fairness_dev,
    //     result.throughput);

    return result;
}





// ticket_lock_t lock; // Declare a test-and-set lock

// int main() {
//     lock_init(&lock); // Initialize the test-and-set lock

//     // Number of threads launched -> will be read from cmd line later
//     int n = 8;

//     // Create counters
//     int count_success[n]; 
//     for (int i = 0; i < n; i++)
//     {
//         count_success[i] = 0;
//     }
 
//     int count_total = 0;
//     int served = 0;

//     // Set the number of threads
//     omp_set_num_threads(n);

//     // Parallel region
//     #pragma omp parallel
//     {
//         int* shared_served = &served;
//         while (count_total < 1000-n) 
//         {
//             // critical_section(count_success, count_total);

//             // Acquire lock
//             lock_acquire(&lock, shared_served);

//             // Critical section
//             // sleepForOneCycle();
//             int tid = omp_get_thread_num();
//             count_success[tid] += 1;
//             // printf("Thread %d is has acquired %d times with ticket %d.\n", tid, count_success[tid], served);
//             count_total += 1;

//             // Release lock
//             lock_release(shared_served);
//         }
//     }

//     for (int i = 0; i < n; i++)
//     {
//         printf("Thread %d: %d / %d\n", i, count_success[i], count_total+1);
//     }
    
//     return 0;
// }