ISIZE=$1
JSIZE=$2

gcc -o c2 c2.c -lm
./c2 $ISIZE $JSIZE

gcc -o c2_openmp c2_openmp.c -lm -fopenmp
./c2_openmp $ISIZE $JSIZE $3
#><

diff result.txt result_omp.txt > /dev/null && echo true || echo false