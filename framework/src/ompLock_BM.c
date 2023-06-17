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
    double threadTic = 0;
    double threadToc = 0;

    while (*successCheck < times) {


        omp_set_lock(OMP_lock);
        threadToc = omp_get_wtime();
        if (threadToc > 0) {
            threadData[id].wait += (threadToc - threadTic);
        }
        (*successCheck)++; //critical Section
        threadData[id].success += 1;
        omp_unset_lock(OMP_lock);
        threadBedtime(sleepCycles); // Take a Nap
        threadTic = omp_get_wtime();
    }
    
}

benchData benchLockOMP(int threads, int times, int sleepCycles) {

    benchData result;
    initializeBenchData(&result);
    threadBenchData thread_data[threads];
    for(int d = 0; d<threads; d++) {
        initializeThreadBenchData(&(thread_data[d]));
    }
    int successCheck = 0;

    omp_lock_t OMP_lock;
    omp_init_lock(&OMP_lock);

    omp_set_dynamic(0); 
    omp_set_num_threads(threads);
    
    double tic, toc;
    tic = omp_get_wtime();

        #pragma omp parallel 
        {
            #pragma omp parallel for
            for (int i=0; i<threads; i++) {
                // &thread_data[0] is pointer to first entry of thread_data array, later used with pointer arithmetic
                threadBench(&OMP_lock, &thread_data[0], &successCheck, times, sleepCycles); 
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

    // printf("OMP Lock Summary: %d Lock acquisiton requests on %d threads took: %f\n",
    //        times, threads, result.time);
    // printf("  with %d failAcq,  %d successAcq, %f %% fairness dev.,  %f  acq/s throughput\n",
    //     result.fail,
    //     result.success,
    //     result.fairness_dev,
    //     result.throughput);

    return result;
}


// int main() {

//     benchLockOMP(2, 1000, 1);
//     benchLockOMP(4, 1000, 1);
//     benchLockOMP(6, 1000, 1);
//     benchLockOMP(8, 1000, 1);
//     benchLockOMP(10, 1000, 1);
//     benchLockOMP(12, 1000, 1);

// }

// gcc -fopenmp ompLock_BM.c benchUtils.c -o ompLock_BM

