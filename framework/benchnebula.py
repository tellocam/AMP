import ctypes
import os
import sys
import numpy as np
import pandas as pd
import utils

# Check if the argument count is correct
if len(sys.argv) != 6:
    print("Usage: python myfile.py [int Threads] [int maxAcq] [int MaxIter] [int Sleepcycles] [string Lock]")
    sys.exit(1)

# Access the argument
maxThreads = int(sys.argv[1])
maxAcq = int(sys.argv[2])
maxIter = int(sys.argv[3])
sleepCyles = int(sys.argv[4])
Lock = str(sys.argv[5])

# set shared library
binary = ctypes.CDLL( "build/sharedLibrary.so" )

lockNameList = ["benchLockOMP", "benchCriticalOMP", "benchTAS", "benchTATAS", 
                "benchTicket", "benchArray", "benchCLH", "benchMCS", "benchHemlock"]

for lock in lockNameList:
    if Lock in lock:
        lockName = lock
        break

print("The matching lock name is:", lockName)

# Load the shared library function
LockC = getattr(binary, lockName)
LockC.restype = utils.benchData

print("Starting Benchmark of " + lockName + "Lock")

threadNum = [i for i in range(2, maxThreads+1)]
bmListLock = {threads: [LockC(threads, maxAcq, sleepCyles) for _ in range(maxIter)] for threads in threadNum}
print("Finish Data Acquisition")
df = utils.dataframeBuilder(bmListLock).fillna(0)
print("Finish DataframeBuilder() call")
df.to_csv("data/"+lockName+ "_THR" + str(maxThreads) +"_ACQ" + str(maxAcq) + "_ITER" + str(maxIter) + '_Nebula.txt', index=False, sep='\t')
print(df)
