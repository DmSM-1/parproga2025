#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


int main(int argc, char** argv){

    MPI_Init(&argc, &argv);

    int commsize, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int ISIZE = atoi(argv[2]);
    int JSIZE = atoi(argv[1]);

    int exec = commsize - 1;
    //количество блоков по три линии
    int num_l3_block = ISIZE/3-1;
    //размер полных подблоков в блоке
    int subblock_hsize = (JSIZE-1)/exec;
    int subblock_size = 3*subblock_hsize;
    //остатки
    int hres_block_vsize = ISIZE%3;
    int vres_block_hsize = (JSIZE-1)%exec;
    
    MPI_Request req[64];
    MPI_Status st[64];
    

    double** buf_ex = calloc(exec, sizeof(double*));
    for(int i = 0; i < exec; i++){
        buf_ex[i] = calloc(subblock_hsize*3, sizeof(double));
    }

    double* buf = calloc(subblock_hsize*3, sizeof(double));

    if (!rank){
        double* a = calloc(ISIZE*JSIZE, sizeof(double));

        for (int i=0; i<ISIZE; i++){
            for (int j=0; j<JSIZE; j++){
                a[i*JSIZE+j] = 10*j+i;
            }
        }

        double* ptr_line = a;

        double begin,end;
        begin = MPI_Wtime();

        //основная часть
        for(int block_num = 0; block_num < num_l3_block; block_num++){
            double* ptr = ptr_line;

            for(int ex = 1; ex<commsize; ex++, ptr+=subblock_hsize){
                memcpy(buf_ex[ex-1], ptr, subblock_hsize*sizeof(double));
                memcpy(buf_ex[ex-1]+subblock_hsize, ptr+JSIZE, subblock_hsize*sizeof(double));
                memcpy(buf_ex[ex-1]+subblock_hsize*2, ptr+JSIZE*2, subblock_hsize*sizeof(double));
                
                MPI_Isend((void*)buf_ex[ex-1], subblock_size, MPI_DOUBLE, ex, 0, MPI_COMM_WORLD, req+ex);
            }

            ptr_line += 3*JSIZE;
            ptr = ptr_line+1;
            for(int ex = 1; ex<commsize; ex++, ptr+=subblock_hsize){
                MPI_Recv((void*)buf, subblock_size, MPI_DOUBLE, ex, 0, MPI_COMM_WORLD, st+ex);
                memcpy(ptr, buf, subblock_hsize*sizeof(double));
                memcpy(ptr+JSIZE, buf+subblock_hsize, subblock_hsize*sizeof(double));
                memcpy(ptr+2*JSIZE, buf+subblock_hsize*2, subblock_hsize*sizeof(double));
            }
        }
        // нижние остатки
        if (hres_block_vsize){
            // ptr_line += 3*JSIZE;
            double* ptr = ptr_line;

            for(int ex = 1; ex<commsize; ex++, ptr+=subblock_hsize){
                memcpy(buf, ptr, subblock_hsize*sizeof(double));
                if (hres_block_vsize == 2)
                    memcpy(buf+subblock_hsize, ptr+JSIZE, subblock_hsize*sizeof(double));

                MPI_Send((void*)buf, subblock_size, MPI_DOUBLE, ex, 0, MPI_COMM_WORLD);
            }

            ptr_line += JSIZE*3;
            ptr = ptr_line+1;
            for(int ex = 1; ex<commsize; ex++, ptr+=subblock_hsize){

                MPI_Recv((void*)buf, subblock_size, MPI_DOUBLE, ex, 0, MPI_COMM_WORLD, st+ex);

                memcpy(ptr, buf, subblock_hsize*sizeof(double));
                if (hres_block_vsize == 2)
                    memcpy(ptr+JSIZE, buf+subblock_hsize, subblock_hsize*sizeof(double));
            }
        }
        // боковые остатки
        for(int i = 3; i < ISIZE; i++){
            for(int j = JSIZE-vres_block_hsize; j < JSIZE; j++){
                a[i*JSIZE+j] = sin(2*a[(i-3)*JSIZE+j-1]);
            }
        }

        end = MPI_Wtime();
        printf("TIME: %lf\n", end-begin);

        FILE* file = fopen("result_mpi.txt","w");
        for(int i = 0; i < JSIZE; i++){
            for (int j= 0; j < ISIZE; j++){
                    fprintf(file,"%f ",a[j*JSIZE+i]);
                }
            fprintf(file,"\n");
        }
        fclose(file);

        free(a);
        for(int i = 0; i < exec; i++){
            buf_ex[i] = calloc(subblock_hsize*3, sizeof(double));
        }
        free(buf_ex);

    }else{
        for(int block_num = 0; block_num < num_l3_block; block_num++){
            MPI_Recv((void*)buf, subblock_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, st);

            for (int i = 0; i < subblock_size; i++)
                buf[i] = sin(2*buf[i]);

            MPI_Send((void*)buf, subblock_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
        if (hres_block_vsize){
            MPI_Recv((void*)buf, subblock_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, st);

            for (int i = 0; i < subblock_size; i++)
                buf[i] = sin(2*buf[i]);

            MPI_Send((void*)buf, subblock_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }
        


    free(buf);


    MPI_Finalize();
}