#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

/* These structs should match the definition in benchmark.py
 */
struct counters {
    int failed_lockAcq;
    int successful_lockAcq;
};
struct bench_result {
    float time;
    struct counters reduced_counters;
};

// Test-and-Set Lock struct
typedef struct {
    int flag;
} TAS_lock_t;

// Initiate a "False" flagged Lock, this means, the lock is NOT acquired by any thread!
void TAS_lock_init(TAS_lock_t* lock) {
    lock->flag = 0;
}


void TAS_lock_acquire(TAS_lock_t* lock, int* fail_counter, int* succ_counter) {
    while (atomic_flag_test_and_set(&(lock->flag))) {
        // Stay in WHILE part until the busy thread sets lock->flag = 0
        // Here we might introduce the non-atomic faileq_lockAcq +=
        
        *fail_counter += 1;
    }
    *succ_counter += 1;

    printf("the succcc is %d\n", *succ_counter);

}

void TAS_lock_release(TAS_lock_t* lock) {
    atomic_flag_clear(&lock->flag);
}

TAS_lock_t lock; // Declare a test-and-set lock

void critical_section(int* fail_counter, int* succ_counter) {

    // Acquire lock
    TAS_lock_acquire(&lock, fail_counter, succ_counter);

    // Critical section
    printf("Thread %d is in the critical section.\n", omp_get_thread_num());
    // int tid = omp_get_thread_num();

    // printf("check the pointer.. %d \n", *succ_counter);


    // Release lock
    TAS_lock_release(&lock);
}


struct counters random_bench1(int times) {

    int tid = omp_get_thread_num();
    printf("Thread %d started.\n", tid);
    // Barrier to force OMP to start all threads at the same time
    #pragma omp barrier

    // struct counters data = {.failed_lockAcq = 0,
    //                         .successful_lockAcq = 0};
    struct counters data;
    data.failed_lockAcq = 0;
    data.successful_lockAcq = 0;

    int* ptr_failed = &(data.failed_lockAcq);
    int* ptr_success = &(data.successful_lockAcq);

    // printf("check the pointer.. %d \n", *ptr_failed);



    for (int i=0; i<times;i++) {
        critical_section(ptr_failed, ptr_success);
    }

    return data;
}

struct bench_result small_bench(int t, int len) { // t is number of threads, len is how many iterations!
    struct bench_result result;
    struct counters thread_data[t];
    double tic, toc;


    omp_set_num_threads(t);
    tic = omp_get_wtime();
    {
        #pragma omp parallel for
        for (int i=0; i<t; i++) {
            thread_data[i] = random_bench1(len);
        }
    }
    toc = omp_get_wtime();

    for (int i=0; i<t; i++) {
        result.reduced_counters.successful_lockAcq += thread_data[i].successful_lockAcq;
        result.reduced_counters.failed_lockAcq     += thread_data[i].failed_lockAcq;
    }

    // printf("wanna check the number of fail: %d\n", result.reduced_counters.failed_lockAcq);
    // printf("wanna check the number of succ: %d\n", result.reduced_counters.successful_lockAcq);

    result.time = (toc - tic);
    printf("Locks: %d Lock acquisiton requests on %d threads took: %f\n",\
           len, t, toc - tic);
    printf("  with %d failed lock acquisitons and %d successful successful lock acquisitions: %f  acquisitions/s throughput\n",\
        result.reduced_counters.failed_lockAcq,
        result.reduced_counters.successful_lockAcq,
        result.reduced_counters.successful_lockAcq/result.time);

    return result;
}

/* main is not relevant for benchmark.py but necessary when run alone for
 * testing.
 */
int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    small_bench(2, 4);
    //small_bench(2, 10);
    //small_bench(4, 10);
    // small_bench(8, 10);
    // small_bench(16, 10);

}

// gcc -fopenmp library.c -o library
