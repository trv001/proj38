#include <pthread.h>
#include <signal.h>

int initThreads(pthread_attr_t *attr){
    if(pthread_attr_init(attr) || pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED)){
            printf("Could not init pthread_attr\n");
            return -1;
    }
    return 1;
}
int attachThreads(pthread_t *threads, pthread_attr_t *attr, flood_args *args, int num){
    for(int j = 0; j < num; j++){
        if(!pthread_create(threads + j, attr, flood, (void*)args);){
                pthread_detach(threads + j);
        }
        else{
            printf("Could not create threads\n");
            return -1;
        }
        pthread_attr_destroy(attr);
        return 1;
    }
}
int killThreads(pthread_t *threads, int num){
    for(int j = 0; j < num; j++){
        pthread_kill(threads + j, 0);
    }
}
