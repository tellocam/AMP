#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h> //
#include <stdbool.h>
#include <omp.h> //
#include <stdatomic.h>
#include <unistd.h> // für sleep()

#include <stddef.h>

struct mcs_node {
    struct mcs_node* next;
    bool locked;
};

// global variable 
_Atomic struct mcs_node* head = NULL;

void lock_init() {
    head = NULL;
    
    
}

void lock_acquire(struct mcs_node* node) {
    // node->next = NULL;
    struct mcs_node* pred = atomic_exchange(&head, node);

    if (pred != NULL) {
        node->locked = true;
        pred->next = node;
        while (node->locked);
    }
}

void lock_release(struct mcs_node* node) {
    if (node->next == NULL) {
        if (atomic_compare_exchange_strong(&head, &node, NULL)) {
            return;
        }
        // Wait for next thread
        while (node->next == NULL);
    }
    node->next->locked = false;
    node->next = NULL;
}

int main() {
    lock_init();

    // Number of threads launched -> will be read from cmd line later
    const int num_threads = 8;
    // const int num_threads = omp_get_max_threads();
    // Set the number of threads
    omp_set_num_threads(num_threads);

     // Create and prepare counters
    int count_success[num_threads]; 
    for (int i = 0; i < num_threads; i++) {count_success[i] = 0;}
    int count_total = 0;

    // Create an array of mcs_node structs to represent multiple threads
    struct mcs_node* nodes = (struct mcs_node*)malloc(num_threads * sizeof(struct mcs_node));
    
    // Acquire and release the lock in parallel using OpenMP's parallel for directive
    #pragma omp parallel for
    for (int i = 0; i < num_threads; i++) {
        struct mcs_node* my_node = &nodes[i];
        lock_acquire(my_node);

        // Critical section protected by the MCS lock
        printf("Thread %d: Acquired critical section\n", omp_get_thread_num());
        // sleep(1);  // Simulating some work inside the critical section
        printf("Thread %d: Exiting critical section\n", omp_get_thread_num());

        lock_release(my_node);
    }

    free(nodes);

    return 0;
}