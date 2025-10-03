import numpy as np
import sys


MATRIX_SIZE = int(sys.argv[1])

MATRIX_A = np.random.random(size=(MATRIX_SIZE, MATRIX_SIZE))*np.random.randint(-255, 255, size=(MATRIX_SIZE, MATRIX_SIZE))
MATRIX_B = np.random.random(size=(MATRIX_SIZE, MATRIX_SIZE))*np.random.randint(-255, 255, size=(MATRIX_SIZE, MATRIX_SIZE))
MATRIX_A = np.array(MATRIX_A, dtype=np.float32)
MATRIX_B = np.array(MATRIX_B, dtype=np.float32)
MATRIX_C = MATRIX_A@MATRIX_B

MATRIX = MATRIX_A.reshape(-1)

with open(sys.argv[2], 'w') as f:
    f.write(f"{MATRIX_SIZE}\n")
    for i in MATRIX:
        f.write(f"{i} ")
    
MATRIX = MATRIX_B.reshape(-1)

with open(sys.argv[3], 'w') as f:
    f.write(f"{MATRIX_SIZE}\n")
    for i in MATRIX:
        f.write(f"{i} ")

MATRIX = MATRIX_C.reshape(-1)

with open(sys.argv[4], 'w') as f:
    f.write(f"{MATRIX_SIZE}\n")
    for i in MATRIX:
        f.write(f"{i} ")

