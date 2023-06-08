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
   _Atomic (struct Node*) head;
   struct Node* node;
};


void lock_init(struct Lock* mcs_lock){
   atomic_store(&mcs_lock->head, (struct Node*) NULL);   
}

void lock_acquire(struct Lock* mcs_lock){
    struct Node* n = (struct Node*)malloc(sizeof(struct Node));
    atomic_store(&n->next, (struct Node*) NULL);
    struct Node* pred = atomic_exchange(&mcs_lock->head, n);

    if (pred != (struct Node*) NULL) {
        atomic_store(&n->locked, true);   
        pred->next = n;
        while (atomic_load(&n->locked)){
            // printf("HELP - %d is prisoned in while loop ACQUIRE\n", omp_get_thread_num());
            // sleep(0.5);
        }
    } 
    else {
        mcs_lock->node = n;
    }
}

void lock_release(struct Lock* mcs_lock)
{
    struct Node* n = mcs_lock->node;
    if (n->next == (struct Node*) NULL){
        // printf("lock_release: if1\n");
        if (atomic_compare_exchange_strong(&mcs_lock->head, &n, (struct Node*) NULL)) {
            // printf("lock_release: if2\n");
            free(n);
            return;
        }
        else {
        // Wait for next thread
            n = mcs_lock->node;
            while (n->next == (struct Node*) NULL) {
                // printf("HELP - %d is prisoned in while loop RELEASE\n", omp_get_thread_num());
                // sleep(1);
            }
        }
    }
    atomic_store(&n->next->locked, false);
    mcs_lock->node = n->next;
    n->next = (struct Node*) NULL;
    free(n);
    // printf("Thread %d: Released lock\n", omp_get_thread_num());
}

void destroy(struct Lock* mcs_lock){
    free(atomic_load(&mcs_lock->head));
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

    // Create an array of mcs_node structs to represent multiple threads
    struct Lock* lock;
    lock = (struct Lock*)malloc(sizeof(struct Lock));
    lock_init(lock);    
    // Acquire and release the lock in parallel using OpenMP's parallel for directive
    #pragma omp parallel for
    for (int i = 0; i < 1000; i++) {
       
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

