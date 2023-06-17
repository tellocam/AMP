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
} TATAS_lock_t;

// Initiate a "False" flagged Lock, this means, the lock is NOT acquired by any thread!
static void lock_init(TATAS_lock_t* lock) {
    lock->flag = 0;
}

static void lock_acquire(TATAS_lock_t* lock, threadBenchData* threadData) {

    int id = omp_get_thread_num();
    double threadTic = omp_get_wtime();
    do {
        while (atomic_load(&lock->flag) == 1){
            threadData[id].fail += 1; 
        }
    } while ( atomic_flag_test_and_set(&lock->flag) );
        // Stay in WHILE part until the busy thread sets lock->flag = 0
        // this thread count here is not correct.. but we dont plot it. ;-)
        
    // printf("Thread %d has left critical section.\n", id);

    double threadToc = omp_get_wtime();
    threadData[id].wait += (threadToc - threadTic);
    threadData[id].success += 1;

}

static void lock_release(TATAS_lock_t* lock) {
    atomic_flag_clear(&lock->flag);
}


static void threadBench(TATAS_lock_t* lock, threadBenchData* threadData, int* successCheck, int times, int sleepCycles) {
    
    while (*successCheck < times) {
        lock_acquire(lock, threadData); // Acquire Lock
        (*successCheck)++; //critical Section
        lock_release(lock); // Release lock
        threadBedtime(sleepCycles); // Take a Nap
    }
    
}

benchData benchTATAS(int threads, int times, int sleepCycles) {

    benchData result;
    initializeBenchData(&result);
    threadBenchData thread_data[threads];
    for(int d = 0; d<threads; d++) {
        initializeThreadBenchData(&(thread_data[d]));
    }
    int successCheck = 0;

    TATAS_lock_t TATAS_lock;
    lock_init(&TATAS_lock);

    omp_set_dynamic(0); 
    omp_set_num_threads(threads);

    double tic, toc;
    tic = omp_get_wtime();

        #pragma omp parallel
        {
            #pragma omp parallel for
            for (int i=0; i<threads; i++) {
                // &thread_data[0] is pointer to first entry of thread_data array, later used with pointer arithmetic
                threadBench(&TATAS_lock, &thread_data[0], &successCheck, times, sleepCycles); 
            }   
        }

    toc = omp_get_wtime();
    result.time = (toc - tic);

    for (int i=0; i<threads; i++) {
        result.success += thread_data[i].success; // total success
        result.fail     += thread_data[i].fail; // total fails
        // result.wait += 1/(double)threads * thread_data[i].wait/(double)thread_data[i].success; // avg wait per thread
        result.wait += thread_data[i].wait/(double)times; // avg wait per thread
        result.fairness_dev += 100 * (abs((double)thread_data[i].success - (double)times/(double)threads) / (double)times); //avg fairness deviation in %
    }

    result.throughput = result.success / result.time;

    // printf("TATAS Lock Summary: %d Lock acquisiton requests on %d threads took: %f\n",
    //        times, threads, result.time);
    // printf("  with %d failAcq,  %d successAcq, %f %% fairness dev.,  %f  acq/s throughput\n",
    //     result.fail,
    //     result.success,
    //     result.fairness_dev,
    //     result.throughput);

    return result;
}


// int main() {

//     benchTATAS(2, 10000, 1);
//     benchTATAS(3, 10000, 1);
//     benchTATAS(4, 10000, 1);
//     benchTATAS(5, 10000, 1);
//     benchTATAS(6, 10000, 1);
//     benchTATAS(7, 10000, 1);
//     benchTATAS(8, 10000, 1);
//     benchTATAS(9, 10000, 1);
//     benchTATAS(10, 10000, 1);
//     benchTATAS(11, 10000, 1);
//     benchTATAS(12, 10000, 1);
// }

// gcc -fopenmp TATAS_BM.c -o TATAS_BM


