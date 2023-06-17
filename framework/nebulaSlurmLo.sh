#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1                 
#SBATCH -c 64
#SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=runLoCont.out

thr=64
acq=1000
iter=250
sc=500

make all
make createVenv

make BenchLockOMP THR=$thr ACQ=$acq ITER=$iter SC=$sc
make BenchCriticalOMP THR=$thr ACQ=$acq ITER=$iter SC=$sc
make BenchTAS THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make BenchTATAS THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make BenchTicket THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make BenchArray THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make BenchMCS THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make BenchCLH THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make BenchHemlock THR=$thr ACQ=$acq ITER=$iter SC=$sc
