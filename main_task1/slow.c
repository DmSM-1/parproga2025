#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define get_time(a) ((double)a.tv_sec+((double)a.tv_nsec)*1e-9)
#define time_dif(a,b) (get_time(b)-get_time(a))

void dot(float* A, float* B, float* D, size_t size){
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            D[i*size+j] = 0;
            for(int k = 0; k < size; k++){
                D[i*size+j] += A[i*size+k]*B[k*size+j];
            }
        }
    }
}


int main(int argc, char** argv){
    
    static struct timespec ts[2];

    int size = 0;
    int len = 0;

    FILE* file_A = fopen(argv[1], "r");
    fscanf(file_A, "%d", &size);

    FILE* file_B = fopen(argv[2], "r");
    fscanf(file_B, "%d", &size);

    len = size*size;

    float* A = malloc(len*sizeof(float));
    float* B = malloc(len*sizeof(float));
    float* D = malloc(len*sizeof(float));

    for(int i = 0; i < len; i++){
        fscanf(file_A, "%f", A+i);
        fscanf(file_B, "%f", B+i);
    }

    fclose(file_A);
    fclose(file_B);

    timespec_get(ts, TIME_UTC);
    dot(A, B, D, size);
    timespec_get(ts+1, TIME_UTC);

    FILE* file_D = fopen(argv[3], "w");

    fprintf(file_D, "%d\n", size);
    for(int i = 0; i < len; i++){
        fprintf(file_D, "%.12f ", D[i]);
    }

    fclose(file_D);

    FILE* file_LOG = fopen(argv[4], "a");
    fprintf(file_LOG, "SLOW:%d %.9lf ", size, time_dif(ts[0], ts[1]));

    fclose(file_LOG);

    free(A);
    free(B);
    free(D);

    return 0;
}