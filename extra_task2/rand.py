import numpy as np


num = 99
lim = 2**16

arr         = np.zeros(num+1, dtype=np.int64)
arr[1::]    = np.random.randint(-lim, lim, size=num, dtype=np.int64)
arr[0]      = num


arr.tofile("data", " ", "%d")