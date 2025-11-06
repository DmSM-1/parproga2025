#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>


#define MIN(a,b) (a<b)?(a):(b) 


void fill_diag(int ISIZE, int JSIZE, int index, const double *buf, double *output) {
    int i = index / JSIZE;
    int j = index % JSIZE;

    int max_down = ISIZE - i - 1;           // сколько строк вниз можно идти
    int max_right = (JSIZE - j - 3) / 3;    // сколько шагов вправо
    int cycles = (max_down < max_right) ? max_down : max_right;
    int hrest = (JSIZE - j - 3) % 3;

    // локальные значения (начальные три)
    double x0 = buf[index];
    double x1 = buf[index + 1];
    double x2 = buf[index + 2];

    // основной цикл — обход диагонали
    for (int k = 0; k < cycles; k++) {
        index += JSIZE + 3;

        x0 = sin(2.0 * x0);
        x1 = sin(2.0 * x1);
        x2 = sin(2.0 * x2);

        output[index]     = x0;
        output[index + 1] = x1;
        output[index + 2] = x2;
    }

    // хвост (если остались неполные 3 значения)
    index += JSIZE + 3;
    for (int g = 0; g < hrest; g++) {
        if (index + g < ISIZE * JSIZE)
            output[index + g] = sin(2.0 * (&x0)[g]);
    }
}
// int fill_diag(int ISIZE, int JSIZE, int index, double* buf, double* output){
//     double local_buf[3] = {0};
//     int full_len = ISIZE * JSIZE;
//     int i = index / JSIZE;
//     int j = index % JSIZE;
//     int cycles = 0;
//     int hrest = 0;
//     if (ISIZE-i-1 < (JSIZE-j-3)/3){
//         cycles = ISIZE-i-1;
//     }else{
//         cycles = (JSIZE-j-3)/3;
//         hrest = (JSIZE-j-3)%3;
//     }

//     memcpy(local_buf, buf+index, 3*sizeof(double));
//     for(int k = 0; k < cycles; k++){
//         index += JSIZE + 3;
//         for(int g = 0; g < 3; g++)
//             local_buf[g] = sin(2*local_buf[g]);
//         memcpy(output+index, local_buf, 3*sizeof(double));
//     }
//     index += JSIZE + 3;
//     for(int g = 0; g < hrest; g++){
//         output[index+g] = sin(2*local_buf[g]);
//     }

    
// }


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
    double* c = NULL;

    int num_idnexes = ISIZE-2+(JSIZE)/3;
    int* indexes = calloc(num_idnexes, sizeof(int));

    for(int i = 0; i < ISIZE-2; i++){
        indexes[i] = (i+1)*JSIZE;
    }
    for(int i = 0; i < (JSIZE)/3; i++){
        indexes[i+ISIZE-2] = i*3;
    }


    double start, end;
    a = calloc(ISIZE * JSIZE, sizeof(double));
    c = calloc(ISIZE * JSIZE, sizeof(double));


    for (int i = 0; i < ISIZE; i++) {
        for (int j = 0; j < JSIZE; j++) {
            a[i * JSIZE + j] = (!i||j<3)?(10 * i + j):-2;
        }
    }


    memcpy(c, a, ISIZE*JSIZE*sizeof(double));
    if (!rank) {
        if (a == NULL) {
            perror("Failed to allocate 'a'");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        start = MPI_Wtime();
    }

    for(int i = rank; i < num_idnexes; i+=commsize)
        fill_diag(ISIZE, JSIZE, indexes[i], a, c);

    MPI_Reduce(
        c,  
        a,
        ISIZE*JSIZE,     
        MPI_DOUBLE,  
        MPI_MAX,   
        0,      
        MPI_COMM_WORLD      
    );

    if (!rank){


        end = MPI_Wtime();
        printf("TIME: %lf\n", end - start);

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
        free(c);


    }
    free(indexes);

    MPI_Finalize();
    return 0;
}