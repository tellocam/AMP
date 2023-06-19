CC = gcc
CFLAGS =  -fopenmp -fPIC
LDFLAGS = -shared -fopenmp -fPIC

SRCDIR = src
INCDIR = include
BUILDDIR = build

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
TARGET = $(BUILDDIR)/sharedLibrary.so

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

clean:
	@rm -f $(OBJS) $(TARGET)

BenchLockOMP:
	@python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) LockOMP

BenchCriticalOMP:
	@python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) CriticalOMP

BenchTAS:
	@python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) benchTAS

BenchTATAS:
	@python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) TATAS

BenchTicket:
	@python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) Ticket

BenchArray:
	@python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) Array

BenchMCS:
	@python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) MCS

BenchCLH:
	@python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) CLH

BenchHemlock:
	@python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) Hemlock

# Here are the contention benchmarks

ContBenchLockOMP:
	@python3 benchContention.py $(THR) $(ACQ) $(ITER) $(SC) LockOMP

ContBenchCriticalOMP:
	@python3 benchContention.py $(THR) $(ACQ) $(ITER) $(SC) CriticalOMP

ContBenchTAS:
	@python3 benchContention.py $(THR) $(ACQ) $(ITER) $(SC) benchTAS

ContBenchTATAS:
	@python3 benchContention.py $(THR) $(ACQ) $(ITER) $(SC) TATAS

ContBenchTicket:
	@python3 benchContention.py $(THR) $(ACQ) $(ITER) $(SC) Ticket

ContBenchArray:
	@python3 benchContention.py $(THR) $(ACQ) $(ITER) $(SC) Array

ContBenchMCS:
	@python3 benchContention.py $(THR) $(ACQ) $(ITER) $(SC) MCS

ContBenchCLH:
	@python3 benchContention.py $(THR) $(ACQ) $(ITER) $(SC) CLH

ContBenchHemlock:
	@python3 benchContention.py $(THR) $(ACQ) $(ITER) $(SC) Hemlock

createVenv:
	@echo -e "\nCreate python virtual environment...\n"
	@python3 -m venv myenv
	
	@echo -e "Activate python virtual environment...\n"
	@. myenv/bin/activate
	
	@echo -e "Upgrade pip and install numpy and matplotlib and pandas...\n"
	
	@pip install --upgrade pip
	@pip install numpy matplotlib pandas

make small-bench:
	@echo -e "\nCreate python virtual environment...\n"
	@python3 -m venv myenv
	
	@echo -e "Activate python virtual environment...\n"
	@. myenv/bin/activate
	
	@echo -e "Upgrade pip and install numpy and matplotlib and pandas...\n"
	@myenv/bin/pip install --upgrade pip
	@myenv/bin/pip install numpy matplotlib pandas
	@echo -e "Running benchmark python script...\n"
	@. myenv/bin/activate
	@srun --cpu-freq=High --time=5:00 -p q_student -N 1 -c 64 python3 benchmake.py 

make small-plot:
	@echo -e "\nCreate python virtual environment...\n"
	@python3 -m venv myenv
	
	@echo -e "Activate python virtual environment...\n"
	@. myenv/bin/activate
	
	@echo -e "Upgrade pip and install numpy and matplotlib and pandas...\n"
	@myenv/bin/pip install --upgrade pip
	@myenv/bin/pip install numpy matplotlib pandas
	@echo -e "Running plotting python script...\n"
	
	@. myenv/bin/activate && python3 plotmake.py