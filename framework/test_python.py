import ctypes
import os
import datetime

class cBenchCounters(ctypes.Structure):
    '''
    This has to match the returned struct in library.c
    '''
    _fields_ = [ ("failed_lockAcq", ctypes.c_int),
                 ("successful_lockAcq", ctypes.c_int) ]

class cBenchResult(ctypes.Structure):
    '''
    This has to match the returned struct in library.c
    '''
    _fields_ = [ ("time", ctypes.c_float),
                 ("counters", cBenchCounters) ]

basedir = os.path.dirname(os.path.abspath(__file__))
binary = ctypes.CDLL( f"{basedir}/library.so" )

binary.small_bench.restype = cBenchResult

new_res = binary.small_bench(10,4)

# print(type(new_res.counters.failed_lockAcq))
# print(new_res.counters.failed_lockAcq)
print(new_res.counters.failed_lockAcq)