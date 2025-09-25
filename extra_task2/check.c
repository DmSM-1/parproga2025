#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>


int main(){
    int a = omp_get_num_procs();

    printf("%d\n", a);
}