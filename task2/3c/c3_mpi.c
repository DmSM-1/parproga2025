#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>


int main(int argc, char** argv){

    MPI_Init(&argc, &argv);

    int commsize, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int ISIZE = atoi(argv[1]);
    int JSIZE = atoi(argv[2]);

    int vrest = ISIZE % commsize;
    int block_vsize = ISIZE / commsize + (rank < vrest);
    
    MPI_Request req;
    MPI_Status st;

    double* local_a = calloc(block_vsize*JSIZE, sizeof(double));
    double* local_b = calloc(block_vsize*JSIZE, sizeof(double));

    if (rank == 0){
        double* a = calloc(ISIZE*JSIZE, sizeof(double));
        double* b = calloc(ISIZE*JSIZE, sizeof(double));
        double begin, end;

        int i,j;
        FILE *ff;

        for (i=0; i<ISIZE; i++){
            for (j=0; j<JSIZE; j++){
                a[i*JSIZE+j] = 10*i+j;
                b[i*JSIZE+j] = 0;
            }
        }

        begin = MPI_Wtime();

        int block_vsize_i = block_vsize; 
        int start = 0;

        for(int i = 1; i < commsize; i++){

            start += block_vsize_i; 
            block_vsize_i = ISIZE / commsize + (i < vrest); 

            MPI_Isend(
                a + start*JSIZE,         
                block_vsize_i*JSIZE,  
                MPI_DOUBLE, 
                i, 
                0, 
                MPI_COMM_WORLD,
                &req
            );
        }

        for (i=0; i<block_vsize; i++){
            for (j = 0; j < JSIZE; j++){
                a[i*JSIZE+j] = sin(0.01*a[i*JSIZE+j]);
            }
        }
        for (i=0; i<block_vsize; i++){
            for (j = 2; j < JSIZE; j++){
                b[i*JSIZE+j] = a[i*JSIZE+j-2]*2.5;
            }
        }

        start = block_vsize;
        for(int i = 1; i < commsize; i++){

            block_vsize_i = ISIZE / commsize + (i < vrest); 
            MPI_Recv(
                b+start*JSIZE,         
                block_vsize_i*JSIZE,  
                MPI_DOUBLE, 
                i, 
                0, 
                MPI_COMM_WORLD,
                &st
            );

            start += block_vsize_i; 
        }

        end = MPI_Wtime();
        printf("TIME: %lf\n", end-begin);
        
        ff = fopen("result_mpi.txt","w");
        for(i= 0; i < ISIZE; i++){
            for (j= 0; j < JSIZE; j++){
                    fprintf(ff,"%f ",b[i*JSIZE+j]);
                }
            fprintf(ff,"\n");
        }

        fclose(ff);
        free(a);
        free(b);

    }else{
        MPI_Recv(
            local_a,         
            block_vsize*JSIZE,  
            MPI_DOUBLE, 
            0, 
            0, 
            MPI_COMM_WORLD,
            &st
        );
        for (int i=0; i<block_vsize; i++){
            for (int j = 0; j < JSIZE; j++){
                local_a[i*JSIZE+j] = sin(0.01*local_a[i*JSIZE+j]);
            }
        }
        for (int i=0; i<block_vsize; i++){
            double buf[2] = {0,0};
            for (int j = 2; j < JSIZE; j++){
                int index = i*JSIZE+j;
                buf[(i*JSIZE+j)%2] = 
                local_b[i*JSIZE+j] = local_a[i*JSIZE+j-2]*2.5;
            }
        }
        MPI_Send(
            local_b,         
            block_vsize*JSIZE,  
            MPI_DOUBLE, 
            0, 
            0, 
            MPI_COMM_WORLD
            // &req
        );
    
    }
    free(local_b);
    free(local_a);

    MPI_Finalize();
}



