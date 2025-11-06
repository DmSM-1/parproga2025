ISIZE=$1
JSIZE=$2

gcc -o c3 c3.c -lm
./c3 $ISIZE $JSIZE

mpicc -o c3_mpi c3_mpi.c -lm
mpirun -np $3 c3_mpi $ISIZE $JSIZE
#=<

# gcc -o c2_openmp c2_openmp.c -lm -fopenmp
# ./c2_openmp $ISIZE $JSIZE $3

diff result.txt result_mpi.txt > /dev/null && echo true || echo false