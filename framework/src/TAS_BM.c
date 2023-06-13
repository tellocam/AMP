#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

#include "../include/benchUtils.h" // Benchmark datatypes and functions

// Test-and-Set Lock struct
typedef struct {
    atomic_int flag;
} TAS_lock_t;

// Initiate a "False" flagged Lock, this means, the lock is NOT acquired by any thread!
static void lock_init(TAS_lock_t* lock) {
    lock->flag = 0;
}

static void lock_acquire(TAS_lock_t* lock, threadBenchData* threadData) {

    int id = omp_get_thread_num();
    double threadTic = omp_get_wtime();

    while (atomic_flag_test_and_set(&(lock->flag))) {
        // Stay in WHILE part until the busy thread sets lock->flag = 0
        threadData[id].fail += 1;
    };
    // printf("Thread %d has left critical section.\n", id);

    double threadToc = omp_get_wtime();
    threadData[id].wait += (threadToc - threadTic);
    threadData[id].success += 1;

}

static void lock_release(TAS_lock_t* lock) {
    atomic_flag_clear(&lock->flag);
}


static void threadBench(TAS_lock_t* lock, threadBenchData* threadData, int* successCheck, int times, int sleepCycles) {
    
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

    TAS_lock_t TAS_lock;
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


int main() {

    // benchTAS(12, 100, 1);
    // benchTAS(12, 100, 5);
    // benchTAS(12, 100, 10);
    // benchTAS(12, 100, 100);
    // benchTAS(12, 100, 1000);
    // benchTAS(12, 100, 10000);
    // benchTAS(12, 100, 100000);
}

// gcc -fopenmp library.c -o library

