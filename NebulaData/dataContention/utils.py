import ctypes
import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def plotStuff(x, y, lock, df, color):
    stdd = "stdd"
    mean = "mean"
    df_error = df[::50]
    plt.plot(df[x], df[mean+y], label = lock, color = color)
    plt.errorbar(df_error[x], df_error[mean+y], df_error[stdd + y], linestyle='None', fmt='o', capsize=3, color = color)

def plotStuff1(x, y, lock, df, color):
    stdd = "stdd"
    mean = "mean"
    df_error = df[::70]
    plt.plot(df[x], df[mean+y], label = lock, color = color)
    

def plotStuffSemilogy(x, y, lock, df, color):
    stdd = "stdd"
    mean = "mean"
    df_error = df[::70]
    plt.semilogy(df[x], df[mean+y], label = lock, color = color)
    plt.errorbar(df_error[x], df_error[mean+y], df_error[stdd + y], linestyle='None', fmt='o', capsize=3, color = color)

def plotStuffLoglog(x, y, lock, df, color):
    stdd = "stdd"
    mean = "mean"
    df_error = df[::70]
    plt.loglog(df[x], df[mean+y], label = lock, color = color)
    plt.errorbar(df_error[x], df_error[mean+y], df_error[stdd + y], linestyle='None', fmt='o', capsize=3, color = color)

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
    
def dataframeBuilder(benchmarkList):
    columnNames = ["threads", "meanTime", "stddTime",
                    "meanFail", "stddFail", "meanWait", "stddWait",
                    "meanFair", "stddFair", "meanTP", "stddTP"]

    # create empty list to store computed values
    data = []

    # iterate more or less smartly over benchmarkList
    for threads, results in benchmarkList.items():

        times = np.array([result.time for result in results])
        fails = np.array([result.fail for result in results])
        waits = np.array([result.wait for result in results])
        fairness = np.array([result.fairness_dev for result in results])
        throughput = np.array([result.throughput for result in results])

        meanTime, stddTime = np.mean(times), np.std(times)
        meanFail, stddFail = np.mean(fails), np.std(fails)
        meanWait, stddWait = np.mean(waits), np.std(waits)
        meanFair, stddFair = np.mean(fairness), np.std(fairness)
        meanTP, stddTP = np.mean(throughput), np.std(throughput)

        data.append([
            threads, meanTime, stddTime,
            meanFail, stddFail, meanWait, stddWait,
            meanFair, stddFair, meanTP, stddTP
        ])

    df = pd.DataFrame(data, columns=columnNames)
    return df

