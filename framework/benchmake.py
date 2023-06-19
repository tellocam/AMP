import ctypes
import os
import sys
import numpy as np
import pandas as pd

import importlib
import utils
importlib.reload(utils)

total_acqs = 640
bench_iters = 35

maxThreads = utils.get_available_threads()
threadNum = []
entry = 0
while entry < maxThreads:
    threadNum.append(entry+2)
    entry += 2
# [2, 4, 6, 8, 10, 12] looks like this!

binary = ctypes.CDLL( "build/sharedLibrary.so" )

binary.benchCriticalOMP.restype = utils.benchData
binary.benchTAS.restype = utils.benchData
binary.benchTATAS.restype = utils.benchData
binary.benchTicket.restype = utils.benchData
binary.benchArray.restype = utils.benchData
binary.benchCLH.restype = utils.benchData
binary.benchMCS.restype = utils.benchData
binary.benchHemlock.restype = utils.benchData

bmListCriticalOMP = {threads: [binary.benchCriticalOMP(threads, total_acqs, 1) for _ in range(bench_iters)] for threads in threadNum}
print("OMP Critical Benchmark done")
bmListTAS = {threads: [binary.benchTAS(threads, total_acqs, 1) for _ in range(bench_iters)] for threads in threadNum}
print("TAS Benchmark done")
bmListTATAS = {threads: [binary.benchTATAS(threads, total_acqs, 1) for _ in range(bench_iters)] for threads in threadNum}
print("TATAS Benchmark done")
bmListTicket = {threads: [binary.benchTicket(threads, total_acqs, 1) for _ in range(bench_iters)] for threads in threadNum}
print("Ticket Benchmark done")
bmListArray = {threads: [binary.benchArray(threads, total_acqs, 1) for _ in range(bench_iters)] for threads in threadNum}
print("Array Benchmark done")
bmListCLH = {threads: [binary.benchCLH(threads, total_acqs, 1) for _ in range(bench_iters)] for threads in threadNum}
print("CLH Benchmark done")
bmListMCS = {threads: [binary.benchMCS(threads, total_acqs, 1) for _ in range(bench_iters)] for threads in threadNum}
print("MCS Benchmark done")
bmListHemlock = {threads: [binary.benchHemlock(threads, total_acqs, 1) for _ in range(bench_iters)] for threads in threadNum}
print("Hemlock Benchmark done")

df_CriticalOMP = utils.dataframeBuilder(bmListCriticalOMP).fillna(0)
df_CriticalOMP.to_csv("data/CriticalOMP.txt", sep='\t', index=False)
df_TAS = utils.dataframeBuilder(bmListTAS).fillna(0)
df_TAS.to_csv("data/TAS.txt", sep='\t', index=False)
df_TATAS = utils.dataframeBuilder(bmListTATAS).fillna(0)
df_TATAS.to_csv("data/TATAS.txt", sep='\t', index=False)
df_Ticket = utils.dataframeBuilder(bmListTicket).fillna(0)
df_Ticket.to_csv("data/Ticket.txt", sep='\t', index=False)
df_Array = utils.dataframeBuilder(bmListArray).fillna(0)
df_Array.to_csv("data/Array.txt", sep='\t', index=False)
df_CLH = utils.dataframeBuilder(bmListCLH).fillna(0)
df_CLH.to_csv("data/CLH.txt", sep='\t', index=False)
df_MCS = utils.dataframeBuilder(bmListMCS).fillna(0)
df_MCS.to_csv("data/MCS.txt", sep='\t', index=False)
df_Hemlock = utils.dataframeBuilder(bmListHemlock).fillna(0)
df_Hemlock.to_csv("data/Hemlock.txt", sep='\t', index=False)

print("All benchmark txt files generated :-)")