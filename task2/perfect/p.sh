ISIZE=$1
JSIZE=$2

gcc -o p p.c -lm
./p $ISIZE $JSIZE

mpicc -o mpi mpi.c -lm 
mpirun -np $3 mpi $ISIZE $JSIZE

gcc -o openmp openmp.c -lm -fopenmp
./openmp $ISIZE $JSIZE $3

diff result.txt result_mpi.txt > /dev/null && echo true || echo false