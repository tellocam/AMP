#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <stdatomic.h>

#include "../include/benchUtils.h" // Benchmark datatypes and functions

void initializeThreadBenchData(threadBenchData* ptr){
    ptr->fail = 0;
    ptr->success = 0;
    ptr->wait = 0;
    ptr->fairness_dev = 0;
}

void initializeBenchData(benchData* ptr){
    ptr->time = 0;
    ptr->fail = 0;
    ptr->success = 0;
    ptr->wait = 0;
    ptr->fairness_dev = 0;
}

void threadBedtime(int sleepCycles) {
    for(int i=0; i<sleepCycles; i++){
        __asm__ volatile("nop");
    };
}