#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>


int main(int argc, char** argv){

    if (argc<3) 
        return -1;

    const int N = atoi(argv[1]);
    const int num_threads = atoi(argv[2]);

    double sum = 0.0;

    omp_set_num_threads(num_threads);

    double start, end;
    
    printf("SLOW VARIANT\n");
    start = omp_get_wtime();
    #pragma omp parallel for reduction(+:sum)
        for(int i = 1; i <= N; i++){
            sum += 1.0 / i;
            // printf("Thread num: %2d | N = %3d | sum = %lf\n",omp_get_thread_num(),i,sum);
        }
    end = omp_get_wtime();

    printf("answer: %lf\ntime: %lf\n", sum, end-start);


    printf("FAST VARIANT\n");
    const int cycles = N / num_threads;
    const int remain = N % num_threads;
    const int whole_part = cycles*num_threads;
    

    sum = 0.0;

    start = omp_get_wtime();
    #pragma omp parallel reduction(+:sum)
    {
        double local_sum = 0.0;
        const int thread_num = omp_get_thread_num();
        for (int i = 0; i < cycles; i++)
            local_sum += 1.0/(thread_num*cycles+i+1);
        if (thread_num < remain)
            local_sum += 1.0/(whole_part+thread_num);
        sum += local_sum;
    }
    end = omp_get_wtime();

    printf("answer: %lf\ntime: %lf\n", sum, end-start);

    return 0;
}