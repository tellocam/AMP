#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h> //
#include <stdbool.h>
#include <omp.h> //
#include <stdatomic.h>
#include <unistd.h> // für sleep()

#include <stddef.h>

struct clh_node {
    struct clh_node* next;
    bool locked;
};

// global variable 
_Atomic struct clh_node* tail;

void lock_init() {
    tail = (_Atomic struct clh_node*)malloc(sizeof(struct clh_node));
    tail->locked = false;
    tail->next = NULL;
    
    
}

void lock_acquire(struct clh_node* my_node) {
    my_node->locked = true;
    my_node->next = atomic_exchange(&tail, my_node);

    while (my_node->next->locked)
    {
        printf("HELP - %d is prisoned in while loop ACQUIRE\n", omp_get_thread_num());
        sleep(0.5);
    }
    
}

void lock_release(struct clh_node* my_node) {
    my_node->next = NULL;
    my_node->locked = false;

    printf("Thread %d: Released lock\n", omp_get_thread_num());
}


int main() {
    // printf("Current issue: (last) lock_release() ends in deadlock\n");
    lock_init();

    // Number of threads launched -> will be read from cmd line later
    const int num_threads = 4;
    // const int num_threads = omp_get_max_threads();
    // Set the number of threads
    omp_set_num_threads(num_threads);

     // Create and prepare counters
    int count_success[num_threads]; 
    for (int i = 0; i < num_threads; i++) {count_success[i] = 0;}
    int count_total = 0;

    // Create an array of clh_node structs to represent multiple threads
    struct clh_node* nodes = (struct clh_node*)malloc(num_threads * sizeof(struct clh_node));
    // struct clh_node* nodes = (struct clh_node*)malloc(2*num_threads * sizeof(struct clh_node));
    
    // Acquire and release the lock in parallel using OpenMP's parallel for directive
    #pragma omp parallel for
    for (int i = 0; i < 32 - num_threads; i++) {
        int tid = omp_get_thread_num();
        struct clh_node* my_node = &nodes[i];
        // struct clh_node* my_node = &nodes[(count_success[tid]%2)+2*i];
        lock_acquire(my_node);

        // Critical section protected by the CLH lock
        // int tid = omp_get_thread_num();
        // printf("Thread %d: Acquired critical section for %d\n", tid, i);
        // sleep(1);  // Simulating some work inside the critical section
        count_success[tid] += 1;
        count_total += 1;
        // printf("Thread %d: Exiting critical section for %d\n", tid, i);

        lock_release(my_node);
    }

    for (int i = 0; i < num_threads; i++)
    {
        printf("Thread %d: %d / %d\n", i, count_success[i], count_total+1);
    }

    // free(count_success);
    free(nodes);

    return 0;
}