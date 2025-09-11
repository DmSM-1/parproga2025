#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


int main(int argc, char** argv){

    if (argc<2) 
        return -1;

    const int num_threads = atoi(argv[1]);
    omp_set_num_threads(num_threads);

    omp_lock_t my_lock;
    omp_init_lock(&my_lock);

    int num = 1;
    int order = 0;

    #pragma omp parallel
    {
        while (1){
            omp_set_lock(&my_lock);
            if (order != omp_get_thread_num()){
                omp_unset_lock(&my_lock);
                continue;
            }
            order++;
            printf("Thread num: %2d | num = %d\n", omp_get_thread_num(), num);
            num *= 2;
            omp_unset_lock(&my_lock);
            break;
        }
    }



    return 0;
}