#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

#include "../include/benchUtils.h" // Benchmark datatypes and functions

// define number of threads
// #define N 8
// define thread local variables
// __thread int mySlot;

typedef struct Array_lock_t{
    bool* flags;
    _Atomic int tail;
} Array_lock_t;

static void lock_init(Array_lock_t* lock) {
    // allocate memory for flags
    int N = omp_get_num_threads();
    lock->flags = malloc(N*sizeof(bool));
    lock->flags[0] = true;
    for (int i=1; i < N; i++){
        lock->flags[i] = false;  
    }
    lock->tail = 0;
}

static void lock_acquire(Array_lock_t* lock, threadBenchData* threadData, int* mySlot) {

    int id = omp_get_thread_num();
    int n = omp_get_num_threads();

    *mySlot = atomic_fetch_add(&lock->tail,1)%n;

    int threadTic = omp_get_wtime();
    while (!lock->flags[*mySlot]) {
        threadData[id].fail += 1;
    };
    int threadToc = omp_get_wtime();

    threadData[id].wait += (threadToc - threadTic);

}

static void lock_release(Array_lock_t* lock, int* mySlot) {

    int N = omp_get_num_threads();

    lock->flags[*mySlot] = false;
    lock->flags[(*mySlot + 1) % N] = true;
}

static void threadBench(Array_lock_t* lock, threadBenchData* threadData,
                        int* successCheck, int times,
                        int sleepCycles, int* mySlot) {

    int id = omp_get_thread_num();
    int n = omp_get_num_threads();
    
    while (*successCheck < times) {

        lock_acquire(lock, threadData, mySlot); // Acquire Lock

        (*successCheck)++; //critical Section
        threadData[id].success += 1;

        lock_release(lock, mySlot); // Release lock

        threadBedtime(sleepCycles); // Take a Nap
    }
    
}

int mySlot;
#pragma omp threadprivate(mySlot)

benchData benchArray(int threads, int times, int sleepCycles) {

    benchData result;
    initializeBenchData(&result);
    threadBenchData thread_data[threads];
    for(int d = 0; d<threads; d++) {
        initializeThreadBenchData(&(thread_data[d]));
    }
    int successCheck = 0;

    Array_lock_t lock;
    lock_init(&lock);

    omp_set_dynamic(0); 

    double tic, toc;
    tic = omp_get_wtime();

        #pragma omp parallel
        omp_set_num_threads(threads); 
        {
            #pragma omp parallel for
            for (int i=0; i<threads; i++) {
                // &thread_data[0] is pointer to first entry of thread_data array, later used with pointer arithmetic
                threadBench(&lock, &thread_data[0], &successCheck, times, sleepCycles, &mySlot); 
            }   
        }

    toc = omp_get_wtime();
    result.time = (toc - tic);

    for (int i=0; i<threads; i++) {
        result.success += thread_data[i].success; // total success
        result.fail     += thread_data[i].fail; // total fails
        result.wait += thread_data[i].wait/((float)times); // avg wait per thread
        result.fairness_dev += 100 * (abs(thread_data[i].success - times/threads) / (float)times); //avg fairness deviation in %
    }

    result.throughput = result.success / result.time;

    // printf("Array Lock Summary: %d Lock acquisiton requests on %d threads took: %f\n",
    //        times, threads, result.time);
    // printf("  with %d failAcq,  %d successAcq, %f %% fairness dev.,  %f  acq/s throughput\n",
    //     result.fail,
    //     result.success,
    //     result.fairness_dev,
    //     result.throughput);

    return result;
}

// int main() {

//     benchArray(2, 10000, 1);
//     benchArray(3, 10000, 1);
//     benchArray(4, 10000, 1);
//     benchArray(5, 10000, 1);
//     benchArray(6, 10000, 1);
//     benchArray(7, 10000, 1);
//     benchArray(8, 10000, 1);
//     benchArray(9, 10000, 1);
//     benchArray(10, 10000, 1);
//     benchArray(11, 10000, 1);
//     benchArray(12, 10000, 1);
// }

// gcc -fopenmp array_BM.c benchUtils.c -o array_BM