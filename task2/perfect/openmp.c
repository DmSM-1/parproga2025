#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

int main(int argc, char **argv){

    int ISIZE = atoi(argv[1]);
    int JSIZE = atoi(argv[2]);
    int executers = atoi(argv[3]);
    double* a = calloc(ISIZE*JSIZE, sizeof(double));
    omp_set_num_threads(executers);

    struct timespec begin, end;
    int i, j;
    FILE *ff;
    for (i=0; i<ISIZE; i++){
        for (j=0; j<JSIZE; j++){
            a[i*JSIZE+j] = 10*i +j;
        }
    }
    //начало измерения времени
    clock_gettime(CLOCK_MONOTONIC, &begin);
    int full_len = ISIZE*JSIZE;
    #pragma omp parallel for schedule(guided)
    for (i=0; i<full_len; i++)
        a[i] = sin(2*a[i]);

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("TIME: %lf\n", end.tv_sec-begin.tv_sec+(end.tv_nsec-begin.tv_nsec)*1e-9);
    //окончание измерения времени
    ff = fopen("result.txt","w");
    for(i=0; i < ISIZE; i++){
        for (j=0; j < JSIZE; j++){
            fprintf(ff,"%f ",a[i*JSIZE+j]);
        }
        fprintf(ff,"\n");
    }
    fclose(ff);
    free(a);
}