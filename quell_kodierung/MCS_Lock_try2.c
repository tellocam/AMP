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

void lock_acquire(struct mcs_node* my_node) {
    struct mcs_node* pred = atomic_exchange(&head, my_node);

    if (pred != NULL) {
        my_node->locked = true;
        pred->next = my_node;
        while (my_node->locked)
        {
            printf("HELP - %d is prisoned in while loop ACQUIRE\n", omp_get_thread_num());
            sleep(0.5);
        }
    }
}

void lock_release(struct mcs_node* my_node) {
    if (my_node->next == NULL) {
        printf("lock_release: if1\n");
        if (atomic_compare_exchange_strong(&head, &my_node, NULL)) {
            printf("lock_release: if2\n");
            return;
        }
        else{
            // Wait for next thread
            while (my_node->next == NULL)
            {
                printf("HELP - %d is prisoned in while loop RELEASE\n", omp_get_thread_num());
                sleep(1);
            }
        }
    }
    else {
        printf("lock_release: not if1\n");
    }
    my_node->next->locked = false;
    my_node->next = NULL;

    // printf("Thread %d: Released lock\n", omp_get_thread_num());
}


int main() {
    printf("Current issue: (last) lock_release() ends in deadlock\n");
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
    for (int i = 0; i < 20 - num_threads; i++) {
        struct mcs_node* my_node = &nodes[i];
        lock_acquire(my_node);

        // Critical section protected by the MCS lock
        int tid = omp_get_thread_num();
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