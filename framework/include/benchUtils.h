#ifndef BENCHUTILS_H
#define BENCHUTILS_H

typedef struct{
    int fail;
    int success;
    float wait;
    float fairness_dev;
} threadBenchData;

typedef struct{
    float time;
    int fail;
    int success;
    float wait;
    float fairness_dev;
    float throughput;
} benchData;

void initializeBenchData(benchData* ptr);

void initializeThreadBenchData(threadBenchData* ptr);

void threadBedtime(int sleepCycles);

#endif // BENCHUTILS_H