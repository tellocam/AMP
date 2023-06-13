#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

#include "framework/include/benchUtils.h"

// Test-and-Set Lock struct
typedef struct {
    atomic_int flag;
} TAS_lock_t;

// Initiate a "False" flagged Lock, this means, the lock is NOT acquired by any thread!
static void lock_init(TAS_lock_t* lock) {
    lock->flag = 0;
}

static void lock_acquire(TAS_lock_t* lock, threadBenchData* thread_statistics) {

    float thread_tic = omp_get_wtime();

    while (atomic_flag_test_and_set(&(lock->flag))) {
        // Stay in WHILE part until the busy thread sets lock->flag = 0
        // Here we might introduce the non-atomic faileq_lockAcq +=
        thread_statistics->fail += 1;
    };

    thread_statistics->wait += omp_get_wtime() - thread_tic;
    thread_statistics->success += 1;

}

static void lock_release(TAS_lock_t* lock) {
    atomic_flag_clear(&lock->flag);
}

static void critical_section(TAS_lock_t* lockFlag, threadBenchData* thread_statistics, int sleepCycles) {

    // Acquire lock

    lock_acquire(lockFlag, thread_statistics);
    threadBedtime(sleepCycles);

    // Critical section
    // printf("Thread %d is in the critical section.\n", omp_get_thread_num());
    // int tid = omp_get_thread_num();
    // printf("check the pointer.. %d \n", *succ_counter);

    // Release lock
    lock_release(lockFlag);
}


static threadBenchData threadBench(TAS_lock_t* lock, int times, int sleepCycles) {

    // int tid = omp_get_thread_num();
    // printf("Thread %d started.\n", tid);
    // Barrier to force OMP to start all threads at the same time
    #pragma omp barrier
    threadBenchData threadData;
    threadData.fail = 0;
    threadData.success = 0;

    for (int i=0; i<times;i++) {
        critical_section(lock, &threadData, sleepCycles);
    }

    return threadData;
}

benchData benchTAS(int threads, int times, int sleepCycles) { // t is number of threads, len is how many iterations!
    benchData result;
    initializeBenchData(&result);
    threadBenchData thread_data[threads];
    for(int d = 0; d<threads; d++) {
        initializeThreadBenchData(&(thread_data[d]));
    }

    TAS_lock_t TAS_lock;
    lock_init(&TAS_lock);

    double tic, toc;
    omp_set_num_threads(threads);
    
    tic = omp_get_wtime();
    {
        #pragma omp parallel for
        for (int i=0; i<threads; i++) {
            thread_data[i] = threadBench(&TAS_lock, times, sleepCycles);
        }
    }

    toc = omp_get_wtime();
    result.time = (toc - tic);

    for (int i=0; i<threads; i++) {
        result.success += thread_data[i].success;
        result.fail     += thread_data[i].fail;
    }

    for (int i = 0; i<threads; i++) {
        result.fairness_dev += thread_data[i].success - (result.success/threads);
        printf("  success for thread %d is %d \n",\
        i,thread_data[i].success);
        // result.fairness_dev,
        // result.throughput);
    }

    result.throughput = result.success / result.time;

    printf("Locks: %d Lock acquisiton requests on %d threads took: %f\n",\
           times, threads, result.time);
    printf("  with %d failed,  %d success, %f fairness dev,  %f  acquisitions/s throughput\n",\
        result.fail,
        result.success,
        result.fairness_dev,
        result.throughput);

    return result;
}

/* main is not relevant for benchmark.py but necessary when run alone for
 * testing.
 */

int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    benchTAS(12, 100, 5);
    // benchTAS(12, 100, 5);
    // benchTAS(12, 100, 10);
    // benchTAS(12, 100, 100);
    // benchTAS(12, 100, 1000);
    // benchTAS(12, 100, 10000);
    // benchTAS(12, 100, 100000);



}

// gcc -fopenmp library.c -o library

