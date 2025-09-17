#include <stdio.h>
#include <stdlib.h>
#include <omp.h>



void par_area(int max_level, int* num_threads){

    int level = omp_get_level();

    if (level < max_level){
        omp_set_num_threads(num_threads[level]);

        #pragma omp parallel
        {
            char buf[256] = {0};
            int ptr = 0;
            
            for(int i = 0; i < level; i++){

                ptr += sprintf(buf+ptr, "[L %2d | td %2d/%d] -> ", 
                    i, 
                    omp_get_ancestor_thread_num(i+1),
                    omp_get_team_size(i+1)
                );
            }

            sprintf(buf+ptr, "[L %2d | td %2d/%d]", 
                    level, 
                    omp_get_thread_num(),
                    omp_get_num_threads()
                );

            printf("%s\n", buf);
                
            par_area(max_level, num_threads);
        }
    }
}


int main(int argc, char** argv){

    if (argc < 3)
        return -1;

    int num_levels = atoi(argv[1]);

    if (num_levels < 1) 
        return -1;

    int* num_threads = calloc(num_levels, sizeof(int));

    for (int i = 0; i < num_levels; i++){
        num_threads[i] = atoi(argv[i+2]);
    }

    omp_set_nested(1);
    omp_set_max_active_levels(num_levels);

    par_area(num_levels, num_threads);

    free(num_threads);

    return 0;
}