#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>




int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int commsize, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 3) {
        if (!rank) {
            printf("Usage: %s ISIZE JSIZE\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int ISIZE = atoi(argv[1]);
    int JSIZE = atoi(argv[2]);

    double* a = NULL;
    double* b = NULL;

    if (!rank) {
        a = calloc(ISIZE * JSIZE, sizeof(double));
        if (a == NULL) {
            perror("Failed to allocate 'a'");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        for (int i = 0; i < ISIZE; i++) {
            for (int j = 0; j < JSIZE; j++) {
                a[i * JSIZE + j] = 10 * i + j;
            }
        }
    }


    int hrest = (JSIZE - 3) % commsize;
    int block_hsize = (JSIZE - 3) / commsize + (rank < hrest);
    MPI_Status st;
    double start, end;

    if (!rank) {
        start = MPI_Wtime();
        int block_hsize_i = block_hsize; 
        int begin = 0; 


        for (int i = 1; i < commsize; i++) {

            begin += block_hsize_i; 
            block_hsize_i = (JSIZE - 3) / commsize + (i < hrest); 
            
            MPI_Send(
                a + begin,         
                block_hsize_i + 3,  
                MPI_DOUBLE, 
                i, 
                0, 
                MPI_COMM_WORLD
            );
        }

        
        int string_len = JSIZE; 
        for (int i = 1; i < ISIZE; i++) {
            for (int j = 0; j < block_hsize; j++) {

                a[i * string_len + j + 3] = sin(2 * a[(i - 1) * string_len + j]);
            }
            
            MPI_Sendrecv(
                a + i * string_len + block_hsize, 
                3,                                
                MPI_DOUBLE,                    
                (commsize > 1 ? 1 : MPI_PROC_NULL),
                0,                               
                NULL,                           
                0,                              
                MPI_DOUBLE,                     
                MPI_PROC_NULL,                 
                0,                              
                MPI_COMM_WORLD,
                &st
            );
        }


        int max_block_hsize = (JSIZE - 3) / commsize;
        if (hrest > 0) max_block_hsize++; 
        
        b = calloc(ISIZE * max_block_hsize, sizeof(double));
        if (b == NULL) {
            perror("Failed to allocate 'b'");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }


        begin = block_hsize + 3; 

        for (int i = 1; i < commsize; i++) {
            block_hsize_i = (JSIZE - 3) / commsize + (i < hrest);
            int packed_recv_size = ISIZE * block_hsize_i;

            MPI_Recv(
                b,                    
                packed_recv_size,      
                MPI_DOUBLE, 
                i, 
                0, 
                MPI_COMM_WORLD, 
                &st
            );

     
            for (int j = 0; j < ISIZE; j++) {
                memcpy(
                    a + j * JSIZE + begin,     
                    b + j * block_hsize_i, 
                    block_hsize_i * sizeof(double)
                );
            }
            begin += block_hsize_i;
        }
        end = MPI_Wtime();
        printf("TIME: %lf\n", end - start);

    } else { 
        int string_len = block_hsize + 3; 
        double* local_buf = calloc(ISIZE * string_len, sizeof(double));
        if (local_buf == NULL) {
            perror("Failed to allocate 'local_buf'");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }


        MPI_Recv(
            local_buf, 
            string_len,
            MPI_DOUBLE, 
            0, 
            0, 
            MPI_COMM_WORLD, 
            &st
        );

        for (int i = 1; i < ISIZE; i++) {

            for (int j = 0; j < block_hsize; j++) {
                local_buf[i * string_len + j + 3] = sin(2 * local_buf[(i - 1) * string_len + j]);
            }
            
            int dest = (rank < commsize - 1) ? rank + 1 : MPI_PROC_NULL;
            int source = rank - 1; 
            
            MPI_Sendrecv(
                local_buf + i * string_len + block_hsize, 
                3,                                      
                MPI_DOUBLE,                             
                dest,                                   
                0,                                     
                local_buf + i * string_len,           
                3,                                
                MPI_DOUBLE,                            
                source,                               
                0,                                      
                MPI_COMM_WORLD,
                &st
            );
        }

        double* packed_buf = malloc(ISIZE * block_hsize * sizeof(double));
        if (packed_buf == NULL) {
            perror("Failed to allocate 'packed_buf'");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        for (int i = 0; i < ISIZE; i++) {

            memcpy(
                packed_buf + i * block_hsize,   
                local_buf + i * string_len + 3,   
                block_hsize * sizeof(double)
            );
        }

        MPI_Send(
            packed_buf, 
            ISIZE * block_hsize,
            MPI_DOUBLE, 
            0, 
            0, 
            MPI_COMM_WORLD
        );
                
        free(local_buf);
        free(packed_buf);
    }
    

    if (!rank) {
        FILE* file = fopen("result_mpi.txt", "w");
        if (file) {
            for (int i = 0; i < ISIZE; i++) {
                for (int j = 0; j < JSIZE; j++) {
                    fprintf(file, "%f ", a[i * JSIZE + j]);
                }
                fprintf(file, "\n");
            }
            fclose(file);
        }
        free(a);
        free(b);
    }

    MPI_Finalize();
    return 0;
}