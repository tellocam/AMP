#include <stdlib.h>
#include <stdatomic.h>
#include <omp.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

struct Node{
   struct Node* next;
   _Atomic bool locked;
   char padding[64];  // avoiding false sharing with the head
   char more_padding[64];  // Additional padding
   
} ;

struct Lock{
   _Atomic (struct Node*) head;
   struct Node* node;
   char padding[64];  // avoiding false sharing with the head
   char more_padding[64];  // Additional padding
};


void lock_init(struct Lock* mcs_lock){
    // atomic_store_explicit(&mcs_lock->head, (struct Node*) NULL, memory_order_relaxed);  
    mcs_lock->head = (struct Node*) NULL;
}

void lock_acquire(struct Lock* mcs_lock){
    struct Node* n = (struct Node*)malloc(sizeof(struct Node));
    // atomic_store_explicit(&n->locked, false, memory_order_relaxed); // newly added
    n->locked = false;
    // atomic_store_explicit(&n->next, (struct Node*) NULL, memory_order_relaxed);
    n->next = (struct Node*) NULL;
    struct Node* pred = atomic_exchange(&mcs_lock->head, n);

    if (pred != (struct Node*) NULL) {
        // atomic_store_explicit(&n->locked, true, memory_order_relaxed);
        n->locked = true;   
        // atomic_store_explicit(&pred->next, n, memory_order_relaxed);
        pred->next = n;
        while (atomic_load(&n->locked)){};
    } 
    else {
        mcs_lock->node = n;
    }
}



void lock_release(struct Lock* mcs_lock)
{
    struct Node* n = atomic_load(&mcs_lock->node);
    if (n->next == (struct Node*) NULL){
        if (atomic_compare_exchange_strong(&mcs_lock->head, &n, (struct Node*) NULL)) {
            free(n);
            return;
        }
        else {
        // Wait for next thread
            n = atomic_load(&mcs_lock->node);
            while (n->next == (struct Node*) NULL) {};
        }
    }
    mcs_lock->node = n->next;
    // atomic_store_explicit(&n->next->locked, false, memory_order_relaxed);
    n->next->locked = false;
    
    n->next = (struct Node*) NULL;
    free(n);
}


int main() {   
    // Number of threads launched -> will be read from cmd line later
    const int num_threads = 6;
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

        printf("Thread %d: Acquired critical section for %d\n", tid, i);
        // sleep(1);  // Simulating some work inside the critical section
        count_success[tid] += 1;
        count_total += 1;
        printf("Thread %d: Exiting critical section for %d\n", tid, i);

        lock_release(lock);
    }

    for (int i = 0; i < num_threads; i++)
    {
        printf("Thread %d: %d / %d\n", i, count_success[i], count_total+1);
    }

    // free head (other nodes already freed in lock_release())
    free(atomic_load(&lock->head));

    return 0;
}

