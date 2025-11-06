ISIZE=$1
JSIZE=$2

gcc -o c1 c1.c -lm
./c1 $ISIZE $JSIZE

# mpicc -o c1_mpi c1_mpi.c -lm
# mpirun -np $3 c1_mpi $ISIZE $JSIZE

# mpicc -o c1_p c1_performance.c -lm
# mpirun -np $3 c1_p $ISIZE $JSIZE
#<<
mpicc -o c1_d c1_diag.c -lm
mpirun -np $3 c1_d $ISIZE $JSIZE


diff result.txt result_mpi.txt > /dev/null && echo true || echo false