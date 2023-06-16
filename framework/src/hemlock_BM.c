#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

#include "../include/benchUtils.h" // Benchmark datatypes and functions

struct Hemlock_Node{
    _Atomic (struct Hemlock*) grant;
   char padding[64];  // avoiding false sharing with the head
} ;

struct Hemlock{
   _Atomic (struct Hemlock_Node*) tail;
   char padding[64];  // avoiding false sharing with the head
};

static __thread struct Hemlock_Node tnode = {(_Atomic (struct Hemlock*)) NULL};

void lock_init(struct Hemlock* hem_lock){
    atomic_store_explicit(&hem_lock->tail, (_Atomic (struct Hemlock_Node*)) NULL, memory_order_relaxed);
}


void lock_acquire(struct Hemlock* hem_lock, threadBenchData* threadData){
    int id = omp_get_thread_num();
    struct Hemlock_Node* n = &tnode;
    n->grant = (struct Hemlock*)NULL;
    
    // Enqueue n at tail of implicit queue
    struct Hemlock_Node* pred = (struct Hemlock_Node*) atomic_exchange(&hem_lock->tail, (_Atomic (struct Hemlock_Node*)) n);

    if (pred != (struct Hemlock_Node*)NULL) {
        // Wait until the current node's grant field is set to NULL
        while(atomic_load(&pred->grant) != hem_lock) {
            threadData[id].fail += 1;
        };
        atomic_store_explicit(&pred->grant, (struct Hemlock*)NULL, memory_order_relaxed);
    }
}

void lock_release(struct Hemlock* hem_lock)
{
    struct Hemlock_Node* n = &tnode;
    n->grant = (struct Hemlock*)NULL;

    // CAS = compare + swap 
    struct Hemlock_Node* v = __sync_val_compare_and_swap(&hem_lock->tail, (_Atomic struct Hemlock_Node*) n, (_Atomic struct Hemlock_Node*) NULL);

    if (v != n){
        // One or more waiters exist -- convey ownership to successor
        atomic_store_explicit(&n->grant, (_Atomic (struct Hemlock*)) hem_lock, memory_order_relaxed);
        while(atomic_load(&n->grant) != ( struct Hemlock*) NULL) {};
    }
}

static void threadBench(struct Hemlock* lock, threadBenchData* threadData,
                        int* successCheck, int times, int sleepCycles) {

    double threadTic = 0;
    double threadToc = 0;                        
    int id = omp_get_thread_num();
    while (*successCheck < times) {
        threadTic = omp_get_wtime();
        lock_acquire(lock, threadData); // Acquire Hemlock
        threadToc = omp_get_wtime();
        threadData[id].wait = threadToc - threadTic;
        threadData[id].success += 1;
        (*successCheck)++; //critical Section
        lock_release(lock); // Release lock
        threadBedtime(sleepCycles); // Take a Nap

    }
    
}

benchData benchHemlock(int threads, int times, int sleepCycles) {

    benchData result;
    initializeBenchData(&result);
    threadBenchData threadData[threads];
    for(int d = 0; d<threads; d++) {
        initializeThreadBenchData(&(threadData[d]));
    }

    int successCheck = 0;

    struct Hemlock* lock;
    lock = (struct Hemlock*)malloc(sizeof(struct Hemlock));
    lock_init(lock);


    double tic, toc;
    tic = omp_get_wtime();
        // omp_set_dynamic(0);
        omp_set_num_threads(threads);
        #pragma omp parallel for schedule(dynamic, 1)
        for (int i=0; i<threads; i++) {
            // &threadData[0] is pointer to first entry of threadData array, later used with pointer arithmetic
            threadBench(lock, &threadData[0], &successCheck, times, sleepCycles);
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

    // printf("MCS Hemlock Summary: %d Hemlock acquisiton requests on %d threads took: %f\n",
    //        times, threads, result.time);
    // printf("  with %d failAcq,  %d successAcq, %f %% fairness dev.,  %f  acq/s throughput\n",
    //     result.fail,
    //     result.success,
    //     result.fairness_dev,
    //     result.throughput);

    atomic_store_explicit(&lock->tail, (struct Hemlock_Node*) NULL, memory_order_relaxed); 

    return result;
}

// int main() {

//     // benchHemlock(2, 10000, 5);
//     benchHemlock(3, 10000, 5);
//     // benchHemlock(4, 10000, 5);
//     benchHemlock(5, 10000, 1);
//     benchHemlock(6, 10000, 1);
//     benchHemlock(7, 10000, 1);
//     benchHemlock(8, 10000, 1);
//     benchHemlock(9, 10000, 1);
//     benchHemlock(10, 10000, 1);
//     benchHemlock(11, 10000, 1);
//     benchHemlock(12, 10000, 1);
// }

// gcc -fopenmp hemlock_BM.c benchUtils.c -o hemlock_BM
