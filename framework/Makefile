CC = gcc
CFLAGS =  -fopenmp -fPIC
LDFLAGS = -shared -fopenmp -fPIC

SRCDIR = src
INCDIR = include
BUILDDIR = build

# THR = 12
# ACQ = 1000
# ITER = 100
# SC = 1

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
	rm -f $(OBJS) $(TARGET)

BenchLockOMP:
	python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) LockOMP

BenchCriticalOMP:
	python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) CriticalOMP

BenchTAS:
	python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) benchTAS

BenchTATAS:
	python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) TATAS

BenchTicket:
	python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) Ticket

BenchArray:
	python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) Array

BenchMCS:
	python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) MCS

BenchCLH:
	python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) CLH

BenchHemlock:
	python3 benchnebula.py $(THR) $(ACQ) $(ITER) $(SC) Hemlock