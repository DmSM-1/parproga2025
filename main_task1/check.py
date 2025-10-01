import numpy as np
import sys


with open(sys.argv[1], 'r') as f:
    size = int(f.readline())
    arr = f.readline().split(' ')

    if '' in arr:
        arr.remove('')

for i in range(size**2):
    arr[i] = float(arr[i])

arr = np.array(arr)
C = arr.reshape(size, size)

with open(sys.argv[2], 'r') as f:
    size = int(f.readline())
    arr = f.readline().split(' ')

    if '' in arr:
        arr.remove('')

for i in range(size**2):
    arr[i] = float(arr[i])

arr = np.array(arr)
R = arr.reshape(size, size)

print(np.max(np.abs(C-R)))