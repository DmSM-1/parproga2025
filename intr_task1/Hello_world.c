#include <stdio.h>
#include <omp.h>

int main() {

    #pragma omp parallel
    {
        printf("Num threads: %2d | thread num %2d | Hello World!\n",  
        omp_get_num_threads(), 
        omp_get_thread_num());
    }

    return 0;
}