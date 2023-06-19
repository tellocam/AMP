import ctypes
import os
import sys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from pylab import rcParams

import importlib
import utils
importlib.reload(utils)

df_CriticalOMP = pd.read_csv("data/CriticalOMP.txt", sep='\t')
df_TAS = pd.read_csv("data/TAS.txt", sep='\t')
df_TATAS = pd.read_csv("data/TATAS.txt", sep='\t')
df_Ticket = pd.read_csv("data/Ticket.txt", sep='\t')
df_Array = pd.read_csv("data/Array.txt", sep='\t')
df_CLH = pd.read_csv("data/CLH.txt", sep='\t')
df_MCS = pd.read_csv("data/MCS.txt", sep='\t')
df_Hemlock = pd.read_csv("data/Hemlock.txt", sep='\t')

rcParams['figure.figsize'] = 10, 10
# plt.rc('text', usetex=True)
# plt.rcParams['text.usetex']=True
# plt.rc('font', family='sans-serif', weight='bold')
SMALL_SIZE = 10
MEDIUM_SIZE = 15
BIGGER_SIZE = 20
plt.rc('font', size=SMALL_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=MEDIUM_SIZE)    # fontsize of the axes title
plt.rc('axes', labelsize=SMALL_SIZE)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=SMALL_SIZE-2)    # legend fontsize
plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title
plt.rcParams['figure.dpi'] = 1000

fig, axes = plt.subplots(nrows=2, ncols=2, figsize=(11,11))
plt.suptitle("Plots for 35 Iterations each with 640 Lock Acquisitons \n Mean values with Standard Deviation errorbars", fontsize=16)

y0 = "meanWait"
e0 = "stddWait"
axes[0,0].plot(df_CriticalOMP["threads"], df_CriticalOMP[y0], label = "OMP Critical", color = "red")
axes[0,0].errorbar(df_CriticalOMP["threads"], df_CriticalOMP[y0], df_CriticalOMP[e0], linestyle='None', fmt='o', capsize=3, color = "red")
axes[0,0].plot(df_TAS["threads"], df_TAS[y0], label = "TAS", color = "blue")
axes[0,0].errorbar(df_TAS["threads"], df_TAS[y0], df_TAS[e0], linestyle='None', fmt='o', capsize=3, color = "blue")
axes[0,0].plot(df_TATAS["threads"], df_TATAS[y0], label = "TATAS", color = "green")
axes[0,0].errorbar(df_TATAS["threads"], df_TATAS[y0], df_TATAS[e0], linestyle='None', fmt='o', capsize=3, color = "green")
axes[0,0].plot(df_Ticket["threads"], df_Ticket[y0], label = "Ticket", color = "orange")
axes[0,0].errorbar(df_Ticket["threads"], df_Ticket[y0], df_Ticket[e0], linestyle='None', fmt='o', capsize=3, color = "orange")
axes[0,0].plot(df_Array["threads"], df_Array[y0], label = "Array", color = "purple")
axes[0,0].errorbar(df_Array["threads"], df_Array[y0], df_Array[e0], linestyle='None', fmt='o', capsize=3, color = "purple")
axes[0,0].plot(df_CLH["threads"], df_CLH[y0], label = "CLH", color = "pink")
axes[0,0].errorbar(df_CLH["threads"], df_CLH[y0], df_CLH[e0], linestyle='None', fmt='o', capsize=3, color = "pink")
axes[0,0].plot(df_MCS["threads"], df_MCS[y0], label = "MCS", color = "#808000")
axes[0,0].errorbar(df_MCS["threads"], df_MCS[y0], df_MCS[e0], linestyle='None', fmt='o', capsize=3, color = "#808000")
axes[0,0].semilogy(df_Hemlock["threads"], df_Hemlock[y0], label = "Hemlock", color = "#4B0082")
axes[0,0].errorbar(df_Hemlock["threads"], df_Hemlock[y0], df_Hemlock[e0], linestyle='None', fmt='o', capsize=3, color = "#4B0082")
axes[0,0].grid(True, which='major', linestyle='-', linewidth=0.5)
axes[0,0].grid(True, which='minor', linestyle='-', linewidth=0.25)
axes[0,0].legend()
axes[0,0].set_xlabel("Threads / -")
axes[0,0].set_ylabel("Latency / $\mathrm{s}$")

y1 = "meanFair"
e1 = "stddFair"
axes[0,1].plot(df_CriticalOMP["threads"], df_CriticalOMP[y1], label = "OMP Critical", color = "red")
axes[0,1].errorbar(df_CriticalOMP["threads"], df_CriticalOMP[y1], df_CriticalOMP[e1], linestyle='None', fmt='o', capsize=3, color = "red")
axes[0,1].plot(df_TAS["threads"], df_TAS[y1], label = "TAS", color = "blue")
axes[0,1].errorbar(df_TAS["threads"], df_TAS[y1], df_TAS[e1], linestyle='None', fmt='o', capsize=3, color = "blue")
axes[0,1].plot(df_TATAS["threads"], df_TATAS[y1], label = "TATAS", color = "green")
axes[0,1].errorbar(df_TATAS["threads"], df_TATAS[y1], df_TATAS[e1], linestyle='None', fmt='o', capsize=3, color = "green")
axes[0,1].plot(df_Ticket["threads"], df_Ticket[y1], label = "Ticket", color = "orange")
axes[0,1].errorbar(df_Ticket["threads"], df_Ticket[y1], df_Ticket[e1], linestyle='None', fmt='o', capsize=3, color = "orange")
axes[0,1].plot(df_Array["threads"], df_Array[y1], label = "Array", color = "purple")
axes[0,1].errorbar(df_Array["threads"], df_Array[y1], df_Array[e1], linestyle='None', fmt='o', capsize=3, color = "purple")
axes[0,1].plot(df_CLH["threads"], df_CLH[y1], label = "CLH", color = "pink")
axes[0,1].errorbar(df_CLH["threads"], df_CLH[y1], df_CLH[e1], linestyle='None', fmt='o', capsize=3, color = "pink")
axes[0,1].plot(df_MCS["threads"], df_MCS[y1], label = "MCS", color = "#808000")
axes[0,1].errorbar(df_MCS["threads"], df_MCS[y1], df_MCS[e1], linestyle='None', fmt='o', capsize=3, color = "#808000")
axes[0,1].plot(df_Hemlock["threads"], df_Hemlock[y1], label = "Hemlock", color = "#4B0082")
axes[0,1].errorbar(df_Hemlock["threads"], df_Hemlock[y1], df_Hemlock[e1], linestyle='None', fmt='o', capsize=3, color = "#4B0082")
axes[0,1].grid(True, which='major', linestyle='-', linewidth=0.5)
axes[0,1].grid(True, which='minor', linestyle='-', linewidth=0.25)
axes[0,1].legend()
axes[0,1].set_xlabel("Threads / -")
axes[0,1].set_ylabel("Fairness Deviation in $\%$ / -")

y3 = "meanTime"
e3 = "stddTime"
axes[1,0].plot(df_CriticalOMP["threads"], df_CriticalOMP[y3], label = "OMP Critical", color = "red")
axes[1,0].errorbar(df_CriticalOMP["threads"], df_CriticalOMP[y3], df_CriticalOMP[e3], linestyle='None', fmt='o', capsize=3, color = "red")
axes[1,0].plot(df_TAS["threads"], df_TAS[y3], label = "TAS", color = "blue")
axes[1,0].errorbar(df_TAS["threads"], df_TAS[y3], df_TAS[e3], linestyle='None', fmt='o', capsize=3, color = "blue")
axes[1,0].plot(df_TATAS["threads"], df_TATAS[y3], label = "TATAS", color = "green")
axes[1,0].errorbar(df_TATAS["threads"], df_TATAS[y3], df_TATAS[e3], linestyle='None', fmt='o', capsize=3, color = "green")
axes[1,0].plot(df_Ticket["threads"], df_Ticket[y3], label = "Ticket", color = "orange")
axes[1,0].errorbar(df_Ticket["threads"], df_Ticket[y3], df_Ticket[e3], linestyle='None', fmt='o', capsize=3, color = "orange")
axes[1,0].plot(df_Array["threads"], df_Array[y3], label = "Array", color = "purple")
axes[1,0].errorbar(df_Array["threads"], df_Array[y3], df_Array[e3], linestyle='None', fmt='o', capsize=3, color = "purple")
axes[1,0].plot(df_CLH["threads"], df_CLH[y3], label = "CLH", color = "pink")
axes[1,0].errorbar(df_CLH["threads"], df_CLH[y3], df_CLH[e3], linestyle='None', fmt='o', capsize=3, color = "pink")
axes[1,0].plot(df_MCS["threads"], df_MCS[y3], label = "MCS", color = "#808000")
axes[1,0].errorbar(df_MCS["threads"], df_MCS[y3], df_MCS[e3], linestyle='None', fmt='o', capsize=3, color = "#808000")
axes[1,0].plot(df_Hemlock["threads"], df_Hemlock[y3], label = "Hemlock", color = "#4B0082")
axes[1,0].errorbar(df_Hemlock["threads"], df_Hemlock[y3], df_Hemlock[e3], linestyle='None', fmt='o', capsize=3, color = "#4B0082")
axes[1,0].grid()
axes[1,0].legend()
axes[1,0].set_xlabel("Threads / -")
axes[1,0].set_ylabel("Elapsed Time /  $\mathrm{s}$")

y2 = "meanTP"
e2 = "stddTP"
axes[1,1].plot(df_CriticalOMP["threads"], df_CriticalOMP[y2], label = "OMP Critical", color = "red")
axes[1,1].errorbar(df_CriticalOMP["threads"], df_CriticalOMP[y2], df_CriticalOMP[e2], linestyle='None', fmt='o', capsize=3, color = "red")
axes[1,1].plot(df_TAS["threads"], df_TAS[y2], label = "TAS", color = "blue")
axes[1,1].errorbar(df_TAS["threads"], df_TAS[y2], df_TAS[e2], linestyle='None', fmt='o', capsize=3, color = "blue")
axes[1,1].plot(df_TATAS["threads"], df_TATAS[y2], label = "TATAS", color = "green")
axes[1,1].errorbar(df_TATAS["threads"], df_TATAS[y2], df_TATAS[e2], linestyle='None', fmt='o', capsize=3, color = "green")
axes[1,1].plot(df_Ticket["threads"], df_Ticket[y2], label = "Ticket", color = "orange")
axes[1,1].errorbar(df_Ticket["threads"], df_Ticket[y2], df_Ticket[e2], linestyle='None', fmt='o', capsize=3, color = "orange")
axes[1,1].plot(df_Array["threads"], df_Array[y2], label = "Array", color = "purple")
axes[1,1].errorbar(df_Array["threads"], df_Array[y2], df_Array[e2], linestyle='None', fmt='o', capsize=3, color = "purple")
axes[1,1].plot(df_CLH["threads"], df_CLH[y2], label = "CLH", color = "pink")
axes[1,1].errorbar(df_CLH["threads"], df_CLH[y2], df_CLH[e2], linestyle='None', fmt='o', capsize=3, color = "pink")
axes[1,1].plot(df_MCS["threads"], df_MCS[y2], label = "MCS", color = "#808000")
axes[1,1].errorbar(df_MCS["threads"], df_MCS[y2], df_MCS[e2], linestyle='None', fmt='o', capsize=3, color = "#808000")
axes[1,1].semilogy(df_Hemlock["threads"], df_Hemlock[y2], label = "Hemlock", color = "#4B0082")
axes[1,1].errorbar(df_Hemlock["threads"], df_Hemlock[y2], df_Hemlock[e2], linestyle='None', fmt='o', capsize=3, color = "#4B0082")
axes[1,1].grid(True, which='major', linestyle='-', linewidth=0.5)
axes[1,1].grid(True, which='minor', linestyle='-', linewidth=0.25)
axes[1,1].legend()
axes[1,1].set_xlabel("Threads / -")
axes[1,1].set_ylabel("Throughput / $\mathrm{Acq}$  $\mathrm{s}^{-1}$")

plt.savefig("plots/small-plot.pdf")
