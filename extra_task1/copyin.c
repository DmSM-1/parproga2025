#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


int x = 0; //if it is not global, error will be
#pragma omp threadprivate(x)

int main(){
    omp_set_num_threads(4);

    x = 10;
    #pragma omp parallel
    {
        x += omp_get_thread_num();
        printf("th %d x=%d\n", omp_get_thread_num(), x);
    }

    printf("\n");


    x = 10;

    #pragma omp parallel copyin(x)
    {
        x += omp_get_thread_num();
        printf("th %d x=%d\n", omp_get_thread_num(), x);
    }

    printf("\n");


        x = 10;

    #pragma omp parallel copyin(x)
    {
        x += omp_get_thread_num();
        #pragma omp single copyprivate(x)
        {
            x = 10;
        }
        printf("th %d x=%d\n", omp_get_thread_num(), x);
    }

    printf("\n");

}