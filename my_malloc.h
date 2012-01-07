#include <unistd.h>
void * my_malloc(size_t n);
void my_free(void * ptr);
#ifndef NDEBUG
void memstats();
#endif
