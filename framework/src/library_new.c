#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

/* These structs should match the definition in benchmark.py
 */

typedef struct {
    int iterations;
    int failed_lockAcq;
    int successful_lockAcq;
    float wait_mean;
    float wait_min;
    float wait_max
    float 
} threadBenchData;

typedef struct{
    threadBenchData reduced_bench;
    float total_time;

} benchData;

// Test-and-Set Lock struct
typedef struct {
    atomic_int flag;
} TAS_lock_t;

// Initiate a "False" flagged Lock, this means, the lock is NOT acquired by any thread!
void TAS_lock_init(TAS_lock_t* lock) {
    lock->flag = 0;
}

void TAS_lock_acquire(TAS_lock_t* lock, threadBenchData* thread_statistics) {

    float thread_tic = omp_get_wtime();

    while (atomic_flag_test_and_set(&(lock->flag))) {
        // Stay in WHILE part until the busy thread sets lock->flag = 0
        // Here we might introduce the non-atomic faileq_lockAcq +=
        thread_statistics->failed_lockAcq += 1;
    };
    thread_statistics->thread_wait = omp_get_wtime() - thread_tic;
    thread_statistics->successful_lockAcq += 1;

    // printf("the succcc is %d\n", *succ_counter);

}

void TAS_lock_release(TAS_lock_t* lock) {
    atomic_flag_clear(&lock->flag);
}

void critical_section(TAS_lock_t* lockFlag, threadBenchData* thread_statistics) {

    // Acquire lock
    TAS_lock_acquire(lockFlag, thread_statistics);
    // Critical section
    printf("Thread %d is in the critical section.\n", omp_get_thread_num());
    // int tid = omp_get_thread_num();
    // printf("check the pointer.. %d \n", *succ_counter);

    // Release lock
    TAS_lock_release(lockFlag);
}


threadBenchData benchPerThread(TAS_lock_t* lock, int times) {

    int tid = omp_get_thread_num();
    printf("Thread %d started.\n", tid);
    // Barrier to force OMP to start all threads at the same time
    #pragma omp barrier
    threadBenchData threadData;
    threadData.failed_lockAcq = 0;
    threadData.successful_lockAcq = 0;

    for (int i=0; i<times;i++) {
        critical_section(lock, &threadData);
    }

    return threadData;
}

benchData bench(TAS_lock_t* lock, int threads, int times) { // t is number of threads, len is how many iterations!
    benchData result;
    threadBenchData thread_data[threads];
    double tic, toc;

    omp_set_num_threads(t);
    tic = omp_get_wtime();
    {
        #pragma omp parallel for
        for (int i=0; i<threads; i++) {
            thread_data[i] = benchPerThread(lock, times);
        }
    }
    toc = omp_get_wtime();

    for (int i=0; i<t; i++) {
        result.reduced_bench.successful_lockAcq += thread_data[i].successful_lockAcq;
        result.reduced_bench.failed_lockAcq     += thread_data[i].failed_lockAcq;
    }

    // printf("wanna check the number of fail: %d\n", result.reduced_counters.failed_lockAcq);
    // printf("wanna check the number of succ: %d\n", result.reduced_counters.successful_lockAcq);

    result.total_time = (toc - tic);
    printf("Locks: %d Lock acquisiton requests on %d threads took: %f\n",\
           times, threads, result.total_time);
    printf("  with %d failed lock acquisitons and %d successful successful lock acquisitions: %f  acquisitions/s throughput\n",\
        result.reduced_bench.failed_lockAcq,
        result.reduced_bench.successful_lockAcq,
        result.reduced_bench.successful_lockAcq/result.total_time);

    return result;
}

/* main is not relevant for benchmark.py but necessary when run alone for
 * testing.
 */
int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    small_bench(6, 4);
    //small_bench(2, 10);
    //small_bench(4, 10);
    // small_bench(8, 10);
    // small_bench(16, 10);

}

// gcc -fopenmp library.c -o library

