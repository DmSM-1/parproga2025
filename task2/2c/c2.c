#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


int main(int argc, char** argv){

    int ISIZE = atoi(argv[1]);
    int JSIZE = atoi(argv[2]);
    double* a = calloc(ISIZE*JSIZE, sizeof(double));
    struct timespec begin, end;

    int i,j;
    FILE *ff;

    for (i=0; i<ISIZE; i++){
        for (j=0; j<JSIZE; j++){
            a[i*JSIZE+j] = 10*i+j;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &begin);
        for (i=0; i<ISIZE-1; i++){
            for (j = 1; j < JSIZE; j++){
                a[i*JSIZE+j] = sin(0.1*a[(i+1)*JSIZE+j-1]);
            }
        }

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("TIME: %lf\n", end.tv_sec-begin.tv_sec+(end.tv_nsec-begin.tv_nsec)*1e-9);
    
    ff = fopen("result.txt","w");
    for(i= 0; i < ISIZE; i++){
        for (j= 0; j < JSIZE; j++){
                fprintf(ff,"%f ",a[i*JSIZE+j]);
            }
        fprintf(ff,"\n");
    }
    fclose(ff);
    free(a);
}



