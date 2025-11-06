#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <string.h>


int main(int argc, char** argv){

    int ISIZE = atoi(argv[1]);
    int JSIZE = atoi(argv[2]);
    int executers = atoi(argv[3]);

    omp_set_num_threads(executers);

    double* a = calloc(ISIZE*JSIZE, sizeof(double));
    double* b = calloc(ISIZE*JSIZE, sizeof(double));
    double start, end;

    int i,j;
    FILE *ff;

    for (i=0; i<ISIZE; i++){
        for (j=0; j<JSIZE; j++){
            a[i*JSIZE+j] = 10*i+j;
        }
    }

    int b_size = (ISIZE-1)*(JSIZE);

    start = omp_get_wtime();
    // memcpy(b, a, ISIZE*JSIZE*sizeof(double));
    int shift = JSIZE-1;

    #pragma omp parallel for schedule(guided)
    for (i = 0; i < b_size; i++){
        b[i] = sin(0.1*a[i+shift]);
    }
    for (i = 0; i < ISIZE; i++){
        b[i*JSIZE] = a[i*JSIZE];
    }
    for (i = 0; i < ISIZE; i++){
        b[b_size+i] = a[b_size+i];
    }



    end = omp_get_wtime();
    printf("TIME: %lf\n", end-start);
    
    ff = fopen("result_omp.txt","w");
    for(i= 0; i < ISIZE; i++){
        for (j= 0; j < JSIZE; j++){
                fprintf(ff,"%f ",b[i*JSIZE+j]);
            }
        fprintf(ff,"\n");
    }
    fclose(ff);
    free(a);
}



