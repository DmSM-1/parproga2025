#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>


double one_to_n_series(int n, double deg){
    double answer = 0.0;

    for (int i = 1; i <= n; i++)
        answer += pow((double)i, -deg);

    return answer;
}

void print_blocks(int cycles, int executers, int* block){
    for (int i = 0; i < executers; i++){
        printf("th: %d blocks: %2d |", i, block[(cycles+1)*i]);
        for(int j = 0; j < block[(cycles+1)*i]; j++){
            printf(" %2d", block[(cycles+1)*i+1+j]);
        }
        printf("\n");
    }
}

int main(int argc, char** argv){
    int cycles = 65;
    int executers = 4;

    if (argc == 2){
        executers = atoi(argv[1]);
    }

    int* block = calloc((cycles+1)*executers, sizeof(int));
    double start, end;

    omp_set_num_threads(executers);

    //static, 1
    for(int i = 0; i < (cycles+1)*executers; i++){
        block[i] = 0;
    }
    start = omp_get_wtime();
    #pragma omp parallel for schedule(static, 1)
    for(int i = 0; i < cycles; i++){
        one_to_n_series(i*10, 1.2345);
        block[(cycles+1)*omp_get_thread_num()] ++;
        block[(cycles+1)*omp_get_thread_num()+block[(cycles+1)*omp_get_thread_num()]] = i;
    }
    end = omp_get_wtime();

    printf("(static,  1): time: %lf\n", end-start);
    if (argc < 2){
    print_blocks(cycles, executers, block);
    printf("\n");
    }


    //static, 4
    for(int i = 0; i < (cycles+1)*executers; i++){
        block[i] = 0;
    }
    start = omp_get_wtime();
    #pragma omp parallel for schedule(static, 4)
    for(int i = 0; i < cycles; i++){
        one_to_n_series(i*10, 1.2345);
        block[(cycles+1)*omp_get_thread_num()] ++;
        block[(cycles+1)*omp_get_thread_num()+block[(cycles+1)*omp_get_thread_num()]] = i;
    }
    end = omp_get_wtime();

    printf("(static,  4): time: %lf\n", end-start);
    if (argc < 2){
    print_blocks(cycles, executers, block);
    printf("\n");
    }


    //dynamic, 1
    for(int i = 0; i < (cycles+1)*executers; i++){
        block[i] = 0;
    }
    start = omp_get_wtime();
    #pragma omp parallel for schedule(dynamic, 1)
    for(int i = 0; i < cycles; i++){
        one_to_n_series(i*10, 1.2345);
        block[(cycles+1)*omp_get_thread_num()] ++;
        block[(cycles+1)*omp_get_thread_num()+block[(cycles+1)*omp_get_thread_num()]] = i;
    }
    end = omp_get_wtime();

    printf("(dynamic, 1): time: %lf\n", end-start);
    if (argc < 2){
    print_blocks(cycles, executers, block);
    printf("\n");
    }


    //dynamic, 4
    for(int i = 0; i < (cycles+1)*executers; i++){
        block[i] = 0;
    }
    start = omp_get_wtime();
    #pragma omp parallel for schedule(dynamic, 4)
    for(int i = 0; i < cycles; i++){
        one_to_n_series(i*10, 1.2345);
        block[(cycles+1)*omp_get_thread_num()] ++;
        block[(cycles+1)*omp_get_thread_num()+block[(cycles+1)*omp_get_thread_num()]] = i;
    }
    end = omp_get_wtime();

    printf("(dynamic, 4): time: %lf\n", end-start);
    if (argc < 2){
    print_blocks(cycles, executers, block);
    printf("\n");
    }


    //guided
    for(int i = 0; i < (cycles+1)*executers; i++){
        block[i] = 0;
    }
    start = omp_get_wtime();
    #pragma omp parallel for schedule(guided)
    for(int i = 0; i < cycles; i++){
        one_to_n_series(i*10, 1.2345);
        block[(cycles+1)*omp_get_thread_num()] ++;
        block[(cycles+1)*omp_get_thread_num()+block[(cycles+1)*omp_get_thread_num()]] = i;
    }
    end = omp_get_wtime();

    printf("(guided    ): time: %lf\n", end-start);
    if (argc < 2){
    print_blocks(cycles, executers, block);
    printf("\n");
    }


    //default
    for(int i = 0; i < (cycles+1)*executers; i++){
        block[i] = 0;
    }
    start = omp_get_wtime();
    #pragma omp parallel for
    for(int i = 0; i < cycles; i++){
        one_to_n_series(i*10, 1.2345);
        block[(cycles+1)*omp_get_thread_num()] ++;
        block[(cycles+1)*omp_get_thread_num()+block[(cycles+1)*omp_get_thread_num()]] = i;
    }
    end = omp_get_wtime();

    printf("(default   ): time: %lf\n", end-start);
    if (argc < 2){
    print_blocks(cycles, executers, block);
    printf("\n");
    }

    return 0;

}