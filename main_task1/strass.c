#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define get_time(a) ((double)a.tv_sec+((double)a.tv_nsec)*1e-9)
#define time_dif(a,b) (get_time(b)-get_time(a))

typedef struct TREE_BF {
    float *M1, *M2, *M3, *M4, *M5, *M6, *M7;
    float *tempA, *tempB;
    struct TREE_BF** branch;
} TREE_BF;

TREE_BF* init_tree(size_t size, size_t threshold) {
    if (size <= threshold) {
        return NULL;
    }

    TREE_BF* node = malloc(sizeof(TREE_BF));
    if (!node) return NULL;

    size_t new_size = size / 2;
    size_t block_len = new_size * new_size;
    
    float* workspace = malloc(9 * block_len * sizeof(float));
    if (!workspace) {
        free(node);
        return NULL;
    }

    node->M1 = workspace;
    node->M2 = node->M1 + block_len;
    node->M3 = node->M2 + block_len;
    node->M4 = node->M3 + block_len;
    node->M5 = node->M4 + block_len;
    node->M6 = node->M5 + block_len;
    node->M7 = node->M6 + block_len;
    node->tempA = node->M7 + block_len;
    node->tempB = node->tempA + block_len;

    node->branch = malloc(7 * sizeof(TREE_BF*));
    if (!node->branch) {
        free(workspace);
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < 7; ++i) {
        node->branch[i] = init_tree(new_size, threshold);
    }

    return node;
}

void free_tree(TREE_BF* node) {
    if (!node) return;

    for (int i = 0; i < 7; ++i) {
        free_tree(node->branch[i]);
    }
    
    free(node->branch);
    free(node->M1);
    free(node);
}


void dot(float* A, float* F, float* D, size_t size, size_t total_size){
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            float sum = 0.0f;
            for(int k = 0; k < size; k++){
                sum += A[i * total_size + k] * F[j * total_size + k];
            }
            D[i * size + j] = sum;
        }
    }
}

void strass(float* A, float* F, float* D, size_t size, size_t total_size, TREE_BF* buffers) {
    if (size <= 256) {
        dot(A, F, D, size, total_size);
        return;
    }

    size_t new_size = size / 2;

    float* A11 = A;
    float* A12 = A + new_size;
    float* A21 = A + new_size * total_size;
    float* A22 = A + new_size * total_size + new_size;

    float* BT11 = F;
    float* BT12 = F + new_size;
    float* BT21 = F + new_size * total_size;
    float* BT22 = F + new_size * total_size + new_size;

    float* M1 = buffers->M1;
    float* M2 = buffers->M2;
    float* M3 = buffers->M3;
    float* M4 = buffers->M4;
    float* M5 = buffers->M5;
    float* M6 = buffers->M6;
    float* M7 = buffers->M7;
    float* tempA = buffers->tempA;
    float* tempB = buffers->tempB;

    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A11[i*total_size+j] + A22[i*total_size+j];
    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT11[i*total_size+j] + BT22[i*total_size+j];
    strass(tempA, tempB, M1, new_size, new_size, buffers->branch[0]);

    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A21[i*total_size+j] + A22[i*total_size+j];
    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT11[i*total_size+j];
    strass(tempA, tempB, M2, new_size, new_size, buffers->branch[1]);

    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A11[i*total_size+j];
    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT21[i*total_size+j] - BT22[i*total_size+j];
    strass(tempA, tempB, M3, new_size, new_size, buffers->branch[2]);

    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A22[i*total_size+j];
    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT12[i*total_size+j] - BT11[i*total_size+j];
    strass(tempA, tempB, M4, new_size, new_size, buffers->branch[3]);

    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A11[i*total_size+j] + A12[i*total_size+j];
    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT22[i*total_size+j];
    strass(tempA, tempB, M5, new_size, new_size, buffers->branch[4]);

    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A21[i*total_size+j] - A11[i*total_size+j];
    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT11[i*total_size+j] + BT21[i*total_size+j];
    strass(tempA, tempB, M6, new_size, new_size, buffers->branch[5]);

    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A12[i*total_size+j] - A22[i*total_size+j];
    for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT12[i*total_size+j] + BT22[i*total_size+j];
    strass(tempA, tempB, M7, new_size, new_size, buffers->branch[6]);
    
    for(int i=0; i < new_size; i++){
        for(int j=0; j < new_size; j++){
            size_t m_idx = i * new_size + j;
            D[(i)*total_size + (j)] = M1[m_idx] + M4[m_idx] - M5[m_idx] + M7[m_idx];
            D[(i)*total_size + (j + new_size)] = M3[m_idx] + M5[m_idx];
            D[(i + new_size)*total_size + (j)] = M2[m_idx] + M4[m_idx];
            D[(i + new_size)*total_size + (j + new_size)] = M1[m_idx] - M2[m_idx] + M3[m_idx] + M6[m_idx];
        }
    }
}

void block_dot(float* A, float* F, float* D, size_t size, size_t total_size) {
    TREE_BF* buffer_tree_root = init_tree(size, 256);
    strass(A, F, D, size, total_size, buffer_tree_root);
    free_tree(buffer_tree_root);
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

    timespec_get(ts, TIME_UTC);
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            F[j*size+i] = B[i*size+j];
        }
    }
    block_dot(A, F, D, size, size);
    
    timespec_get(ts+1, TIME_UTC);

    FILE* file_D = fopen(argv[3], "w");

    fprintf(file_D, "%d\n", size);
    for(int i = 0; i < len; i++){
        fprintf(file_D, "%.12f ", D[i]);
    }

    fclose(file_D);

    FILE* file_LOG = fopen(argv[4], "a");
    fprintf(file_LOG, "STRAS:%d %.9lf ", size, time_dif(ts[0], ts[1]));

    fclose(file_LOG);

    free(A);
    free(B);
    free(F);
    free(D);

    return 0;
}