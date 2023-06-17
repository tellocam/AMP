#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1                 
#SBATCH -c 64
#SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=runHiCont.out

thr=64
acq=1000
iter=1000
sc=500

make all
make createVenv

# make ContBenchLockOMP THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make ContBenchCriticalOMP THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make ContBenchTAS THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make ContBenchTATAS THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make ContBenchTicket THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make ContBenchArray THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make ContBenchMCS THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make ContBenchCLH THR=$thr ACQ=$acq ITER=$iter SC=$sc
# make ContBenchHemlock THR=$thr ACQ=$acq ITER=$iter SC=$sc
