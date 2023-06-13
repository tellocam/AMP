#include <stdio.h>
#include <omp.h>

int main() {
    int i;
    int num_threads = 4;

    // Set the number of threads
    omp_set_num_threads(num_threads);

    #pragma omp parallel
    {
        // Get the thread ID
        int tid = omp_get_thread_num();

        // Get the total number of threads
        int num_threads = omp_get_num_threads();

        printf("Hello from thread %d of %d\n", tid, num_threads);

        // Parallel for loop
        #pragma omp for
        for (i = 0; i < 10; i++) {
            printf("Thread %d: i = %d\n", tid, i);
        }
    }

    return 0;
}