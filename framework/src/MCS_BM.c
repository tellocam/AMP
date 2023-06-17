#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

#include "../include/benchUtils.h" // Benchmark datatypes and functions

struct MCS_Node{
   struct MCS_Node* next;
   _Atomic bool locked;
   char padding[64];  // avoiding false sharing with the head
   char more_padding[64];  // Additional padding
} ;

struct MCS_Lock_t{
   _Atomic (struct MCS_Node*) head;
   struct MCS_Node* node;
   char padding[64];  // avoiding false sharing with the head
   char more_padding[64];  // Additional padding
};

static void lock_init(struct MCS_Lock_t* mcs_lock){
    mcs_lock->head = (struct MCS_Node*) NULL;
}

static void lock_acquire(struct MCS_Lock_t* mcs_lock, threadBenchData* threadData){

    int id = omp_get_thread_num();
    struct MCS_Node* n = (struct MCS_Node*)malloc(sizeof(struct MCS_Node));

    n->locked = false;
    n->next = (struct MCS_Node*) NULL;

    struct MCS_Node* pred = atomic_exchange(&mcs_lock->head, n);

    if (pred != (struct MCS_Node*) NULL) {
        n->locked = true;   
        pred->next = n;

        while (atomic_load(&n->locked)){
            threadData[id].fail += 1;
        };
    } 
    else {
        mcs_lock->node = n;
    }

}



static void lock_release(struct MCS_Lock_t* mcs_lock)
{
    struct MCS_Node* n = atomic_load(&mcs_lock->node);
    if (n->next == (struct MCS_Node*) NULL){
        if (atomic_compare_exchange_strong(&mcs_lock->head, &n, (struct MCS_Node*) NULL)) {
            free(n);
            return;
        }
        else {
        // Wait for next thread
            n = atomic_load(&mcs_lock->node);
            while (n->next == (struct MCS_Node*) NULL) {};
        }
    }
    mcs_lock->node = n->next;
    n->next->locked = false;
    
    n->next = (struct MCS_Node*) NULL;
    free(n);
}

static void threadBench(struct MCS_Lock_t* lock, threadBenchData* threadData,
                        int* successCheck, int times, int sleepCycles) {

    double threadTic = 0;
    double threadToc = 0;                        
    int id = omp_get_thread_num();
    while (*successCheck < times) {
        threadTic = omp_get_wtime();
        lock_acquire(lock, threadData); // Acquire MCS_Lock_t
        threadToc = omp_get_wtime();
        threadData[id].wait = threadToc - threadTic;
        threadData[id].success += 1;
        (*successCheck)++; //critical Section
        lock_release(lock); // Release lock
        threadBedtime(sleepCycles); // Take a Nap


    }
    
}

benchData benchMCS(int threads, int times, int sleepCycles) {

    benchData result;
    initializeBenchData(&result);
    threadBenchData threadData[threads];
    for(int d = 0; d<threads; d++) {
        initializeThreadBenchData(&(threadData[d]));
    }

    int successCheck = 0;

    struct MCS_Lock_t* lock;
    lock = (struct MCS_Lock_t*)malloc(sizeof(struct MCS_Lock_t));
    lock_init(lock);
 
    double tic, toc;
    tic = omp_get_wtime();

        #pragma omp parallel
        omp_set_dynamic(0); 
        omp_set_num_threads(threads);
        {
            #pragma omp parallel for
            for (int i=0; i<threads; i++) {
                // &threadData[0] is pointer to first entry of threadData array, later used with pointer arithmetic
                threadBench(lock, &threadData[0], &successCheck, times, sleepCycles); 
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

    // printf("MCS Lock Summary: %d Lock acquisiton requests on %d threads took: %f\n",
    //        times, threads, result.time);
    // printf("  with %d failAcq,  %d successAcq, %f %% fairness dev.,  %f  acq/s throughput\n",
    //     result.fail,
    //     result.success,
    //     result.fairness_dev,
    //     result.throughput);

    return result;
}

// int main() {

//     benchMCS(2, 10000, 5);
//     benchMCS(3, 10000, 5);
//     benchMCS(4, 10000, 5);
//     benchMCS(5, 10000, 5);
//     benchMCS(6, 10000, 5);
//     benchMCS(7, 10000, 5);
//     benchMCS(8, 10000, 5);
//     benchMCS(9, 10000, 5);
//     benchMCS(10, 10000, 5);
//     benchMCS(11, 10000, 5);
//     benchMCS(12, 10000, 5);
// }

// gcc -fopenmp MCS_BM.c benchUtils.c -o MCS_BM