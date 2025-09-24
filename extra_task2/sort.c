#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

static int* buf = NULL;
static int task_size;

void merge_sort(int* arr, int left, int right){
    int len = right - left + 1;
    if (len>1){
        int mean = left + (right - left)/2;

        #pragma omp task if(len>=task_size)
        merge_sort(arr, left, mean);

        #pragma omp task if(len>=task_size)
        merge_sort(arr, mean+1, right);

        #pragma omp taskwait

        int a = left;
        int b = mean + 1;
        int i = 0;

        while (a <= mean && b <= right) {
            if (arr[a] < arr[b]){
                buf[left+i] = arr[a];
                a++;
            }else{
                buf[left+i] = arr[b];
                b++;
            }
            i++;
        }

        for (;a <= mean; i++, a++){
            buf[left+i] = arr[a];
        }
        for (;b <= right; i++, b++){
            buf[left+i] = arr[b];
        }

        memcpy(arr+left, buf+left, len*sizeof(int));
    }
}

int main(int argc, char** argv){
    size_t len = 0;
    double start, end;
    task_size = atoi(argv[3]);
    
    FILE* file = fopen(argv[1], "r");
    fscanf(file, "%lu", &len);
    int* numbers = malloc(len*sizeof(int));
    buf = malloc(len*sizeof(int));

    for (size_t i = 0; i < len; i++){
        fscanf(file, "%d", numbers+i);
    }
    fclose(file);

    start = omp_get_wtime();
    #pragma omp parallel
    #pragma omp single
    merge_sort(numbers, 0, len-1);
    end = omp_get_wtime();

    file = fopen(argv[2], "w");
    for (size_t i = 0; i < len; i++){
        fprintf(file, "%d ", numbers[i]);
    }
    fclose(file);

    printf("%lf\n", end-start);

    free(buf);
    free(numbers);
    return 0;
}