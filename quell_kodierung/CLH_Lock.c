#include <stdlib.h>
#include <stdatomic.h>
#include <omp.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

struct Node{
   struct Node* next;
   _Atomic bool locked;
   char padding[64];
   
} ;

struct Lock{
   _Atomic (struct Node*) tail;
   struct Node* node;
   char padding[64];  // avoiding false sharing with the tail
};


void lock_init(struct Lock* clh_lock){

    struct Node* n = (struct Node*) malloc(sizeof(struct Node));
    // atomic_store_explicit(&n->locked, false, memory_order_relaxed);
    n->locked = false;

    clh_lock->node = n;
    // atomic_store_explicit(&clh_lock->tail, n, memory_order_relaxed);
    clh_lock->tail = n;
}

void lock_acquire(struct Lock* clh_lock){
    struct Node* n = (struct Node*) malloc(sizeof(struct Node));
    // atomic_store_explicit(&n->locked, true, memory_order_relaxed);
    n->locked = true;
    n->next = atomic_exchange(&clh_lock->tail, n);
    while (atomic_load(&n->next->locked)) {
        // printf("HELP - %d is prisoned in while loop ACQUIRE\n", omp_get_thread_num());
        // sleep(0.5);
    }

    clh_lock->node = n;
}

void lock_release(struct Lock* clh_lock)
{
    // atomic_store(&clh_lock->node->locked, false);
    clh_lock->node->locked = false;
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
    for (int i = 0; i < 100000 - 1; i++) {
       
        lock_acquire(lock);

        // Critical section protected by the MCS lock
        int tid = omp_get_thread_num();
        // printf("Thread %d: Acquired critical section for %d\n", tid, i);
        // sleep(1);  // Simulating some work inside the critical section
        count_success[tid] += 1;
        count_total += 1;
        // printf("Thread %d: Exiting critical section for %d\n", tid, i);

        lock_release(lock);
    }

    for (int i = 0; i < num_threads; i++)
    {
        printf("Thread %d: %d / %d\n", i, count_success[i], count_total+1);
    }

    // free head (other nodes already freed in lock_release())
    free(lock->tail);

    return 0;
}

