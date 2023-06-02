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
    int failed_turns;
    int successful_lends;
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
    while (atomic_flag_test_and_set(&lock->flag)) {
        // Stay in WHILE part until the busy thread sets lock->flag = 0
        // Here we might introduce the non-atomic faileq_lockAcq +=
        fail_counter++;
    }
    
    succ_counter++;
    

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
    int tid = omp_get_thread_num();

    // Release lock
    TAS_lock_release(&lock);
}


struct counters random_bench1(int times) {

    int tid = omp_get_thread_num();
    printf("Thread %d started.\n", tid);
    // Barrier to force OMP to start all threads at the same time
    #pragma omp barrier

    // struct counters data = {.failed_turns = 0,
    //                         .successful_lends = 0};
    struct counters data;
    data.failed_turns = 0;
    data.successful_lends = 0;


    int* data_ptr_ft = &(data.failed_turns);
    int* data_ptr_sl = &(data.successful_lends);



    for (int i=0; i<times;i++) {

        critical_section(data_ptr_ft, data_ptr_sl);
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
        result.reduced_counters.successful_lends += thread_data[i].successful_lends;
        result.reduced_counters.failed_turns     += thread_data[i].failed_turns;
    }

    result.time = (toc - tic);
    printf("LIBRARY: lending %d books on %d threads took: %fs\n",\
           len, t, toc - tic);
    printf("  with %d failed turns and %d successful lends: %f lends/s throughput\n",\
        result.reduced_counters.failed_turns,
        result.reduced_counters.successful_lends,
        result.reduced_counters.successful_lends/result.time);

    return result;
}

/* main is not relevant for benchmark.py but necessary when run alone for
 * testing.
 */
int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    small_bench(2, 10);
    // small_bench(2, 10);
    // small_bench(4, 10);
    // small_bench(8, 10);
    // small_bench(16, 10);
}
