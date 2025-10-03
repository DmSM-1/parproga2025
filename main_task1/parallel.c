#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <omp.h>

#define get_time(a) ((double)a.tv_sec+((double)a.tv_nsec)*1e-9)
#define time_dif(a,b) (get_time(b)-get_time(a))

typedef struct TREE_BF {
    float *M[7];
    float *tempA[7];
    float *tempB[7];
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
    
    float* workspace = calloc(21 * block_len, sizeof(float));
    if (!workspace) {
        free(node);
        return NULL;
    }

    node->M[0] = workspace;
    for (int i = 1; i < 7; ++i) node->M[i] = node->M[i-1] + block_len;
    node->tempA[0] = node->M[6] + block_len;
    for (int i = 1; i < 7; ++i) node->tempA[i] = node->tempA[i-1] + block_len;
    node->tempB[0] = node->tempA[6] + block_len;
    for (int i = 1; i < 7; ++i) node->tempB[i] = node->tempB[i-1] + block_len;

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
    free(node->M[0]);
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

    float* M[7];
    float* tA[7];
    float* tB[7];
    for (int i = 0; i < 7; ++i) {
        M[i] = buffers->M[i];
        tA[i] = buffers->tempA[i];
        tB[i] = buffers->tempB[i];
    }

    int use_tasks = (size >= 512);

    if (use_tasks) {
        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            float* tempA = tA[0];
            float* tempB = tB[0];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A11[i*total_size+j] + A22[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT11[i*total_size+j] + BT22[i*total_size+j];
            strass(tempA, tempB, M[0], new_size, new_size, buffers->branch[0]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            float* tempA = tA[1];
            float* tempB = tB[1];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A21[i*total_size+j] + A22[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT11[i*total_size+j];
            strass(tempA, tempB, M[1], new_size, new_size, buffers->branch[1]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            float* tempA = tA[2];
            float* tempB = tB[2];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A11[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT21[i*total_size+j] - BT22[i*total_size+j];
            strass(tempA, tempB, M[2], new_size, new_size, buffers->branch[2]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            float* tempA = tA[3];
            float* tempB = tB[3];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A22[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT12[i*total_size+j] - BT11[i*total_size+j];
            strass(tempA, tempB, M[3], new_size, new_size, buffers->branch[3]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            float* tempA = tA[4];
            float* tempB = tB[4];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A11[i*total_size+j] + A12[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT22[i*total_size+j];
            strass(tempA, tempB, M[4], new_size, new_size, buffers->branch[4]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            float* tempA = tA[5];
            float* tempB = tB[5];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A21[i*total_size+j] - A11[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT11[i*total_size+j] + BT21[i*total_size+j];
            strass(tempA, tempB, M[5], new_size, new_size, buffers->branch[5]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            float* tempA = tA[6];
            float* tempB = tB[6];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A12[i*total_size+j] - A22[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT12[i*total_size+j] + BT22[i*total_size+j];
            strass(tempA, tempB, M[6], new_size, new_size, buffers->branch[6]);
        }

        #pragma omp taskwait
    } else {
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[0][i*new_size+j] = A11[i*total_size+j] + A22[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[0][i*new_size+j] = BT11[i*total_size+j] + BT22[i*total_size+j];
        strass(tA[0], tB[0], M[0], new_size, new_size, buffers->branch[0]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[1][i*new_size+j] = A21[i*total_size+j] + A22[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[1][i*new_size+j] = BT11[i*total_size+j];
        strass(tA[1], tB[1], M[1], new_size, new_size, buffers->branch[1]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[2][i*new_size+j] = A11[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[2][i*new_size+j] = BT21[i*total_size+j] - BT22[i*total_size+j];
        strass(tA[2], tB[2], M[2], new_size, new_size, buffers->branch[2]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[3][i*new_size+j] = A22[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[3][i*new_size+j] = BT12[i*total_size+j] - BT11[i*total_size+j];
        strass(tA[3], tB[3], M[3], new_size, new_size, buffers->branch[3]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[4][i*new_size+j] = A11[i*total_size+j] + A12[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[4][i*new_size+j] = BT22[i*total_size+j];
        strass(tA[4], tB[4], M[4], new_size, new_size, buffers->branch[4]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[5][i*new_size+j] = A21[i*total_size+j] - A11[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[5][i*new_size+j] = BT11[i*total_size+j] + BT21[i*total_size+j];
        strass(tA[5], tB[5], M[5], new_size, new_size, buffers->branch[5]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[6][i*new_size+j] = A12[i*total_size+j] - A22[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[6][i*new_size+j] = BT12[i*total_size+j] + BT22[i*total_size+j];
        strass(tA[6], tB[6], M[6], new_size, new_size, buffers->branch[6]);
    }

    for(int i=0; i < new_size; i++){
        for(int j=0; j < new_size; j++){
            size_t m_idx = i * new_size + j;
            D[(i)*total_size + (j)] = M[0][m_idx] + M[3][m_idx] - M[4][m_idx] + M[6][m_idx];
            D[(i)*total_size + (j + new_size)] = M[2][m_idx] + M[4][m_idx];
            D[(i + new_size)*total_size + (j)] = M[1][m_idx] + M[3][m_idx];
            D[(i + new_size)*total_size + (j + new_size)] = M[0][m_idx] - M[1][m_idx] + M[2][m_idx] + M[5][m_idx];
        }
    }
}

void block_dot(float* A, float* F, float* D, size_t size, size_t total_size) {
    TREE_BF* buffer_tree_root = init_tree(size, 256);
    #pragma omp task shared(A,F,D,buffer_tree_root) firstprivate(size,total_size)
    strass(A, F, D, size, total_size, buffer_tree_root);
    #pragma omp taskwait
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
    float* D = calloc(len, sizeof(float));

    for(int i = 0; i < len; i++){
        fscanf(file_A, "%f", A+i);
        fscanf(file_B, "%f", B+i);
    }

    fclose(file_A);
    fclose(file_B);

    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            F[j*size+i] = B[i*size+j];
        }
    }

    omp_set_nested(1);
    omp_set_num_threads(omp_get_max_threads());

    timespec_get(ts, TIME_UTC);
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            block_dot(A, F, D, size, size);
        }
    }
    timespec_get(ts+1, TIME_UTC);

    FILE* file_D = fopen(argv[3], "w");

    fprintf(file_D, "%d\n", size);
    for(int i = 0; i < len; i++){
        fprintf(file_D, "%.12f ", D[i]);
    }

    fclose(file_D);

    FILE* file_LOG = fopen(argv[4], "a");
    fprintf(file_LOG, "parallel:%d %.9lf ", size, time_dif(ts[0], ts[1]));

    fclose(file_LOG);

    free(A);
    free(B);
    free(F);
    free(D);

    return 0;
}
