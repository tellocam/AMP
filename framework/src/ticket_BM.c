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

static void lock_acquire(ticket_lock_t* lock, int *served, threadBenchData* threadData) {

    int id = omp_get_thread_num();

    int next = atomic_fetch_add_explicit(&lock->ticket,1, 0);
    double threadTic = omp_get_wtime();
    
    while (*served < next) {
        // Thread within spin!
        threadData[id].fail += 1;
    };

    double threadToc = omp_get_wtime();
    threadData[id].wait += (threadToc - threadTic);
}

static void lock_release(int* served) {
    #pragma omp atomic
    (*served)++;
}

static void threadBench(ticket_lock_t* lock, threadBenchData* threadData,
                        int* successCheck, int times, int sleepCycles,
                        int *served) {

    int id = omp_get_thread_num();
    while (*successCheck < times) {

        lock_acquire(lock, served, threadData); // Acquire Lock
        threadData[id].success += 1;
        (*successCheck)++; //critical Section
        lock_release(served); // Release lock
        threadBedtime(sleepCycles); // Take a Nap

    }
    
}

benchData benchTicket(int threads, int times, int sleepCycles) {

    benchData result;
    initializeBenchData(&result);
    threadBenchData threadData[threads];
    for(int d = 0; d<threads; d++) {
        initializeThreadBenchData(&(threadData[d]));
    }

    int successCheck = 0;

    ticket_lock_t lock;
    lock_init(&lock);
    int served = 0;

    omp_set_dynamic(0); 
    omp_set_num_threads(threads);

    double tic, toc;
    tic = omp_get_wtime();

        #pragma omp parallel
        {
            #pragma omp parallel for
            for (int i=0; i<threads; i++) {
                // &threadData[0] is pointer to first entry of threadData array, later used with pointer arithmetic
                threadBench(&lock, &threadData[0], &successCheck, times, sleepCycles, &served); 
            }   
        }

    toc = omp_get_wtime();
    result.time = (toc - tic);

    for (int i=0; i<threads; i++) {
        result.success += threadData[i].success; // total success
        result.fail     += threadData[i].fail; // total fails
        // result.wait += 1/(double)threads * threadData[i].wait/(double)threadData[i].success; // avg wait per thread
        result.wait += threadData[i].wait/(double)times; // avg wait per thread
        result.fairness_dev += 100 * (abs((double)threadData[i].success - (double)times/(double)threads) / (double)times); //avg fairness deviation in %
    }

    result.throughput = result.success / result.time;

    // printf("Ticket Lock Summary: %d Lock acquisiton requests on %d threads took: %f\n",
    //        times, threads, result.time);
    // printf("  with %d failAcq,  %d successAcq, %f %% fairness dev.,  %f  acq/s throughput\n",
    //     result.fail,
    //     result.success,
    //     result.fairness_dev,
    //     result.throughput);

    return result;
}

// int main() {

//     benchTicket(2, 10000, 5);
//     benchTicket(3, 10000, 5);
//     benchTicket(4, 10000, 5);
//     benchTicket(5, 10000, 5);
//     benchTicket(6, 10000, 5);
//     benchTicket(7, 10000, 5);
//     benchTicket(8, 10000, 5);
//     benchTicket(9, 10000, 5);
//     benchTicket(10, 10000, 5);
//     benchTicket(11, 10000, 5);
//     benchTicket(12, 10000, 5);
// }

// gcc -fopenmp ticket_BM.c benchUtils.c -o ticket_BM