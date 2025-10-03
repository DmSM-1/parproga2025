#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define get_time(a) ((double)a.tv_sec+((double)a.tv_nsec)*1e-9)
#define time_dif(a,b) (get_time(b)-get_time(a))
#define THRESHOLD 256

typedef struct MEM_TREE
{
    struct MEM_TREE* branch;
    float* MEM;
    float* sub_M[4];

    size_t size;
    size_t len;
    size_t _2size;
    size_t _4len;

}MEM_TREE;



void* MEM_init(MEM_TREE* tree, size_t size){
    if (size < THRESHOLD || size%2){
        return NULL;
    }

    tree->size = size;
    tree->_2size = size/2;
    tree->len  = size*size;
    tree->_4len  = size*size/4;

    tree->branch = calloc(8, sizeof(MEM_TREE));
    tree->MEM    = calloc(tree->len, sizeof(float));

    tree->sub_M[0] = tree->MEM;
    tree->sub_M[1] = tree->MEM + tree->_4len;
    tree->sub_M[2] = tree->MEM + tree->_4len*2;
    tree->sub_M[3] = tree->MEM + tree->_4len*3;


    for (int i = 0; i < 8; i++){
        MEM_init(tree->branch+i, size/2);
    }
}


void* MEM_free(MEM_TREE* tree, size_t size){
    if (size < THRESHOLD || size%2){
        return NULL;
    }

    free(tree->MEM);
    
    for (int i = 0; i < 8; i++){
        MEM_free(tree->branch+i, size/2);
    }

    free(tree->branch);
}


void dot(float* A, float* B, float* D, size_t size, size_t total_size){
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            float sum = 0.0f;
            for(int k = 0; k < size; k++){
                sum += A[i*total_size+k]*B[j*total_size+k];
            }
            D[i*size+j] += sum;
        }
    }
}


void add_to(float* A, float* B, size_t A_size, size_t B_size, size_t y, size_t x){
    int index = B_size*y+x;
    for(int i = 0; i < A_size; i++){
        for(int j = 0; j < A_size; j++){
             B[i*B_size+index+j] += A[i*A_size+j];
        }
    }
}


void block_dot(float* A, float* B, float* C, size_t size, size_t total_size, MEM_TREE* tree){
        size_t new_size = size / 2;
        
        if (size < THRESHOLD || size%2){
            dot(A, B, C, size, total_size);
        }else{
            float* A11 = A;
            float* A12 = A + new_size;
            float* A21 = A + new_size * total_size;
            float* A22 = A + new_size * total_size + new_size;

            float* B11 = B;
            float* B12 = B + new_size;
            float* B21 = B + new_size * total_size;
            float* B22 = B + new_size * total_size + new_size;

            block_dot(A11, B11, tree->sub_M[0], new_size, total_size, tree->branch);
            block_dot(A12, B12, tree->sub_M[0], new_size, total_size, tree->branch+1);
            add_to(tree->sub_M[0], C, new_size, size, 0, 0);

            block_dot(A11, B21, tree->sub_M[1], new_size, total_size, tree->branch+2);
            block_dot(A12, B22, tree->sub_M[1], new_size, total_size, tree->branch+3);
            add_to(tree->sub_M[1], C, new_size, size, 0, new_size);

            block_dot(A21, B11, tree->sub_M[2], new_size, total_size, tree->branch+4);
            block_dot(A22, B12, tree->sub_M[2], new_size, total_size, tree->branch+5);
            add_to(tree->sub_M[2], C, new_size, size, new_size, 0);

            block_dot(A21, B21, tree->sub_M[3], new_size, total_size, tree->branch+6);
            block_dot(A22, B22, tree->sub_M[3], new_size, total_size, tree->branch+7);
            add_to(tree->sub_M[3], C, new_size, size, new_size, new_size);

            // free(MEM);
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
    float* F = malloc(len*sizeof(float));
    float* D = malloc(len*sizeof(float));

    for(int i = 0; i < len; i++){
        fscanf(file_A, "%f", A+i);
        fscanf(file_B, "%f", B+i);
    }

    fclose(file_A);
    fclose(file_B);

    MEM_TREE tree;
    MEM_init(&tree, size);

    timespec_get(ts, TIME_UTC);
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            F[i*size+j] = B[j*size+i];
        }
    }
    block_dot(A, F, D, size, size, &tree);
    timespec_get(ts+1, TIME_UTC);

    MEM_free(&tree, size);

    FILE* file_D = fopen(argv[3], "w");

    fprintf(file_D, "%d\n", size);
    for(int i = 0; i < len; i++){
        fprintf(file_D, "%.12f ", D[i]);
    }

    fclose(file_D);

    FILE* file_LOG = fopen(argv[4], "a");
    fprintf(file_LOG, "BLOCK+TRANS:%d %.9lf ", size, time_dif(ts[0], ts[1]));

    fclose(file_LOG);

    free(A);
    free(B);
    free(F);
    free(D);

    return 0;
}