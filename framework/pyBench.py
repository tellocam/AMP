import ctypes
import os
import pandas as pd
import numpy as np

class benchData(ctypes.Structure):
    '''
    This has to match the returned struct in library.c
    '''
    _fields_ = [ ("time", ctypes.c_double),
                 ("fail", ctypes.c_int),
                 ("success", ctypes.c_int),
                 ("wait", ctypes.c_double),
                 ("fairness_dev", ctypes.c_double),
                 ("throughput", ctypes.c_double), ]


binary = ctypes.CDLL( "build/sharedLibrary.so" )
binary.benchTAS.restype = benchData

#benchTAS("numberOfThreads", "totalLockAcquisitions", "nonCriticalSleepCycles")
benchmarkList = [[]]
for threads in range(1, 11): 
    benchmarkList.append([threads])
    for iters in range(100):
        benchmarkList[threads].append(binary.benchTAS(threads+1, 100, 1))

for threads in range(1,11):
    print(benchmarkList[threads][50].wait)

# print(type(new_res.counters.failed_lockAcq))
# print(new_res.counters.failed_lockAcq)
# print(new_res.counters.failed_lockAcq)