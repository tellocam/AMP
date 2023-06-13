import ctypes
import os
import numpy as np
import pandas as pd
import utils as benchUtils


maxThreads = 12

binary = ctypes.CDLL( "build/sharedLibrary.so" )
binary.benchTAS.restype = benchUtils.benchData

threadNum = [i for i in range(2, maxThreads+1)]
benchmarkList = {threads: [binary.benchTAS(threads, 100, 1) for _ in range(100)] for threads in threadNum}

df = benchUtils.dataframeBuilder(benchmarkList)
print(df)
