int initThreads(pthread_attr_t*);
int attachThreads(pthread_t*, pthread_attr_t*, flood_args*, int);
int killThreads(pthread_t*, int);
