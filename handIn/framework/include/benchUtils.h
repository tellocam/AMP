#ifndef BENCHUTILS_H
#define BENCHUTILS_H

typedef struct{
    int fail;
    int success;
    double wait;
    double fairness_dev;
} threadBenchData;

typedef struct{
    double time;
    int fail;
    int success;
    double wait;
    double fairness_dev;
    double throughput;
} benchData;

void initializeBenchData(benchData* ptr);

void initializeThreadBenchData(threadBenchData* ptr);

void threadBedtime(int sleepCycles);

#endif // BENCHUTILS_H