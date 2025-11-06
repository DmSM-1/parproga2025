#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>


int main(int argc, char **argv){
    MPI_Init(&argc, &argv);
    int i, j;
    FILE *ff;

    int ISIZE = atoi(argv[1]);
    int JSIZE = atoi(argv[2]);

    double start, end;

    
    int commsize, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int vrest = (ISIZE) % commsize;
    int block_vsize = (ISIZE) / commsize + (rank < vrest);
    MPI_Status st;
    
    double* a = calloc(ISIZE*JSIZE, sizeof(double));
    double* local_buf = calloc(JSIZE*block_vsize, sizeof(double));
    int* margin = calloc(commsize, sizeof(int));
    int* block_size_i = calloc(commsize, sizeof(int));

    block_size_i[0] = block_vsize*JSIZE;
    for (int ex = 1; ex < commsize; ex++){
        margin[ex] += block_size_i[ex-1]+margin[ex-1]; 
        block_size_i[ex]= ((ISIZE) / commsize + (ex < vrest))*JSIZE; 
    }

    if (!rank){
        for (i=0; i<ISIZE; i++){
            for (j=0; j<JSIZE; j++){
                a[i*JSIZE+j] = 10*i +j;
            }
        }
        start = MPI_Wtime();
    }

    MPI_Scatterv(
        a,           
        block_size_i,  
        margin,      
        MPI_DOUBLE,  
        local_buf,  
        block_vsize * JSIZE, 
        MPI_DOUBLE,  
        0,           
        MPI_COMM_WORLD
    );

    for (i=0; i<block_vsize; i++){
        for (j = 0; j < JSIZE; j++){
            local_buf[i*JSIZE+j] = sin(2*local_buf[i*JSIZE+j]);
        }
    }

    MPI_Gatherv(
        local_buf,  
        block_vsize * JSIZE, 
        MPI_DOUBLE,  
        a,           
        block_size_i,  
        margin,      
        MPI_DOUBLE,  
        0,          
        MPI_COMM_WORLD
    );

    if (!rank){
        end = MPI_Wtime();
        printf("TIME: %lf\n", end - start);
        
        ff = fopen("result_mpi.txt","w");
        for(i=0; i < ISIZE; i++){
            for (j=0; j < JSIZE; j++){
                fprintf(ff,"%f ",a[i*JSIZE+j]);
            }
            fprintf(ff,"\n");
        }
        fclose(ff);
    }
    
    free(a);
    free(local_buf);
    free(margin);
    free(block_size_i);
    MPI_Finalize();
}