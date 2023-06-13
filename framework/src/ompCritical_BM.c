#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

#include "../include/benchUtils.h" // Benchmark datatypes and functions

static void threadBench(omp_lock_t* OMP_lock, threadBenchData* threadData, int* successCheck, int times, int sleepCycles) {
    int id = omp_get_thread_num();
    double threadTic = omp_get_wtime();

    while (*successCheck < times) {

        #pragma omp critical 
        {
            (*successCheck)++; //critical Section
            threadData[id].success += 1;
        }
        
        threadBedtime(sleepCycles); // Take a Nap
    }
    
}

benchData benchCriticalOMP(int threads, int times, int sleepCycles) {

    benchData result;
    initializeBenchData(&result);
    threadBenchData thread_data[threads];
    for(int d = 0; d<threads; d++) {
        initializeThreadBenchData(&(thread_data[d]));
    }
    int successCheck = 0;

    omp_lock_t OMP_lock;
    omp_init_lock(&OMP_lock);

    double tic, toc;

    #pragma omp parallel
    omp_set_num_threads(threads); 

    tic = omp_get_wtime();
        #pragma omp parallel for
        for (int i=0; i<threads; i++) {
            // &thread_data[0] is pointer to first entry of thread_data array, later used with pointer arithmetic
            threadBench(&OMP_lock, &thread_data[0], &successCheck, times, sleepCycles); 
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

    printf("TAS Lock Summary: %d Lock acquisiton requests on %d threads took: %f\n",
           times, threads, result.time);
    printf("  with %d failAcq,  %d successAcq, %f %% fairness dev.,  %f  acq/s throughput\n",
        result.fail,
        result.success,
        result.fairness_dev,
        result.throughput);

    return result;
}


// int main() {

//     benchCriticalOMP(2, 1000, 1);
//     benchCriticalOMP(4, 1000, 1);
//     benchCriticalOMP(6, 1000, 1);
//     benchCriticalOMP(8, 1000, 1);
//     benchCriticalOMP(10, 1000, 1);
//     benchCriticalOMP(12, 1000, 1);

// }

// gcc -fopenmp library.c -o library

