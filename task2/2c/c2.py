import numpy as np
import matplotlib.pyplot as plt
import sys
import subprocess


ISIZE = [1000]
JSIZE = [1000]

N = np.arange(1,12)

average = 50

T = np.zeros(len(N))


for j in N:
    for i in range(average):
        result = subprocess.run(f"./c2_openmp {ISIZE[0]} {JSIZE[0]} {j}".split(' '), capture_output=True, text=True)
        # print(f"./c1_mpi {ISIZE[0]} {JSIZE[0]} {j}")
        T[j-1] += float(result.stdout.split(' ')[-1])
    T[j-1] /= average

T/=T[0]
T = 1/T
plt.plot(np.arange(1,len(T)+1), T)
plt.show()
