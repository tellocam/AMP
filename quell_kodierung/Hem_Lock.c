#include <stdlib.h>
#include <stdatomic.h>
#include <omp.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>


struct Node{
    _Atomic (struct Lock*) grant;
   char padding[64];  // avoiding false sharing with the head
} ;

struct Lock{
   _Atomic (struct Node*) tail;
   char padding[64];  // avoiding false sharing with the head
};

__thread struct Node tnode = {(_Atomic (struct Lock*)) NULL};



void lock_init(struct Lock* hem_lock){
    atomic_store_explicit(&hem_lock->tail, (_Atomic (struct Node*)) NULL, memory_order_relaxed);
}


void lock_acquire(struct Lock* hem_lock){
    struct Node* n = &tnode;
    n->grant = (struct Lock*)NULL;
    
    // Enqueue n at tail of implicit queue
    struct Node* pred = (struct Node*) atomic_exchange(&hem_lock->tail, (_Atomic (struct Node*)) n);

    if (pred != (struct Node*)NULL) {
        // Wait until the current node's grant field is set to NULL
        while(atomic_load(&pred->grant) != hem_lock) {};
        atomic_store_explicit(&pred->grant, (struct Lock*)NULL, memory_order_relaxed);
    }
}

void lock_release(struct Lock* hem_lock)
{
    struct Node* n = &tnode;
    n->grant = (struct Lock*)NULL;

    // CAS = compare + swap 
    struct Node* v = __sync_val_compare_and_swap(&hem_lock->tail, (_Atomic struct Node*) n, (_Atomic struct Node*) NULL);

    if (v != n){
        // One or more waiters exist -- convey ownership to successor
        atomic_store_explicit(&n->grant, (_Atomic (struct Lock*)) hem_lock, memory_order_relaxed);
        while(atomic_load(&n->grant) != ( struct Lock*) NULL) {};
    }
}


int main() {   
    // Number of threads launched -> will be read from cmd line later
    const int num_threads = 8;
    // const int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);

     // Create and prepare counters
    int count_success[num_threads]; 
    for (int i = 0; i < num_threads; i++) {count_success[i] = 0;}
    int count_total = 0;

    // Create and allocate lock structure
    struct Lock* lock;
    lock = (struct Lock*)malloc(sizeof(struct Lock));
    lock_init(lock);    
    // Acquire and release the lock in parallel using OpenMP's parallel for directive
    #pragma omp parallel for
    for (int i = 0; i < 1000000 - 1; i++) {
        int tid = omp_get_thread_num();
        // printf("Thread %d: Started procedure for %d\n", tid, i);
        lock_acquire(lock);

        // Critical section protected by the MCS lock
         tid = omp_get_thread_num();
        // printf("Thread %d: Acquired critical section for %d\n", tid, i);
        // sleep(1);  // Simulating some work inside the critical section
        count_success[tid] += 1;
        count_total += 1;
        // printf("Thread %d: Exiting critical section for %d\n", tid, i);

        lock_release(lock);
        // printf("Thread %d: Released for %d\n", tid, i);

    
    }

    for (int i = 0; i < num_threads; i++)
    {
        printf("Thread %d: %d / %d\n", i, count_success[i], count_total+1);
    }

    // free head (other nodes already freed in lock_release())
    atomic_store_explicit(&lock->tail, (struct Node*) NULL, memory_order_relaxed); 
    // atomic_store_explicit(&lock->node, (struct Node*) NULL, memory_order_relaxed);  
    return 0;
}
