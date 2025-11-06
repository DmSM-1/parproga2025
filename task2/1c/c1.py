import numpy as np
import matplotlib.pyplot as plt
import sys
import subprocess


ISIZE = [1000]
JSIZE = [1000]

N = np.arange(2,12)

average = 10

T = np.zeros(len(N)+1)

simple_time = 0
for i in range(average):
    result = subprocess.run(f"./c1 {ISIZE[0]} {JSIZE[0]}".split(' '), capture_output=True, text=True)
    T[0] += float(result.stdout.split(' ')[-1])
T[0] /= average

for j in N:
    for i in range(average):
        result = subprocess.run(f"mpirun -np {j} c1_d {ISIZE[0]} {JSIZE[0]}".split(' '), capture_output=True, text=True)
        # print(f"./c1_mpi {ISIZE[0]} {JSIZE[0]} {j}")
        T[j-1] += float(result.stdout.split(' ')[-1])
    T[j-1] /= average

T/=T[0]
T = 1/T
plt.plot(np.arange(1,len(T)+1), T)
plt.show()
