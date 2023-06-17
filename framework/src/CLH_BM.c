#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

#include "../include/benchUtils.h" // Benchmark datatypes and functions

struct CLH_Node{
   struct CLH_Node* next;
   _Atomic bool locked;
   char padding[64];
};

struct CLH_Lock_t{
   _Atomic (struct CLH_Node*) tail;
   struct CLH_Node* node;
   char padding[64];  // avoiding false sharing with the tail
};


static void lock_init(struct CLH_Lock_t* clh_lock){

    struct CLH_Node* n = (struct CLH_Node*) malloc(sizeof(struct CLH_Node));
    n->locked = false;

    clh_lock->node = n;
    clh_lock->tail = n;
}

static void lock_acquire(struct CLH_Lock_t* clh_lock, threadBenchData* threadData){
    int id = omp_get_thread_num();
    struct CLH_Node* n = (struct CLH_Node*) malloc(sizeof(struct CLH_Node));
    n->locked = true;
    n->next = atomic_exchange(&clh_lock->tail, n);
    // double threadTic = omp_get_wtime();
    while (atomic_load(&n->next->locked)) {
        threadData[id].fail += 1;
    };
    // double threadToc = omp_get_wtime();
    // threadData[id].wait += (threadToc - threadTic);
    
    clh_lock->node = n;
}

static void lock_release(struct CLH_Lock_t* clh_lock)
{
    clh_lock->node->locked = false;
}

static void threadBench(struct CLH_Lock_t* lock, threadBenchData* threadData,
                        int* successCheck, int times, int sleepCycles) {
    

    double threadTic, threadToc;
    int id = omp_get_thread_num();
    while (*successCheck < times) {

        threadTic = omp_get_wtime();
        lock_acquire(lock, threadData); // Acquire CLH_Lock_t
        threadToc = omp_get_wtime();
        threadData[id].wait += (threadToc - threadTic);
        threadData[id].success += 1;
        (*successCheck)++; //critical Section
        lock_release(lock); // Release lock
        threadBedtime(sleepCycles); // Take a Nap

    }
    
}

benchData benchCLH(int threads, int times, int sleepCycles) {

    benchData result;
    initializeBenchData(&result);
    threadBenchData threadData[threads];
    for(int d = 0; d<threads; d++) {
        initializeThreadBenchData(&(threadData[d]));
    }

    int successCheck = 0;

    struct CLH_Lock_t lock;
    lock_init(&lock);

    omp_set_dynamic(0); 
    omp_set_num_threads(threads);

    double tic, toc;
    tic = omp_get_wtime();

        #pragma omp parallel
        {
            #pragma omp parallel for
            for (int i=0; i<threads; i++) {
                // &threadData[0] is pointer to first entry of threadData array, later used with pointer arithmetic
                threadBench(&lock, &threadData[0], &successCheck, times, sleepCycles); 
            }
        }

    toc = omp_get_wtime();
    result.time = (toc - tic);

    for (int i=0; i<threads; i++) {
        result.success += threadData[i].success; // total success
        result.fail     += threadData[i].fail; // total fails
        // result.wait += 1/(double)threads * threadData[i].wait/((double)threadData[i].success); // avg wait per thread
        result.wait += threadData[i].wait/(double)times; // avg wait per thread
        // printf("I wait for: %f\n", result.wait);
        result.fairness_dev += 100 * (abs((double)threadData[i].success - (double)times/(double)threads) / (double)times); //avg fairness deviation in %
    }

    result.throughput = result.success / result.time;

    // printf("CLH Lock Summary: %d Lock acquisiton requests on %d threads took: %f\n",
    //        times, threads, result.time);
    // printf("  with %d failAcq,  %d successAcq, %f %% fairness dev.,  %f  acq/s throughput\n",
    //     result.fail,
    //     result.success,
    //     result.fairness_dev,
    //     result.throughput);

    return result;
}

// int main() {

//     benchCLH(2, 10000, 1);
//     benchCLH(3, 10000, 1);
//     benchCLH(4, 10000, 1);
//     benchCLH(5, 10000, 1);
//     benchCLH(6, 10000, 1);
//     benchCLH(7, 10000, 1);
//     benchCLH(8, 10000, 1);
//     benchCLH(9, 10000, 1);
//     benchCLH(10, 10000, 1);
//     benchCLH(11, 10000, 1);
//     benchCLH(12, 10000, 1);
// }

// gcc -fopenmp CLH_BM.c benchUtils.c -o CLH_BM