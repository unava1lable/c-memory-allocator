#ifndef _MEMORY_ALLOC_T
#define _MEMORY_ALLOC_T

#include <unistd.h>

struct header_t {
    size_t size; // usable memory
    int is_free; // 0 for false; 1 for true
    struct header_t *next; // next free memory block
}__attribute__((aligned(16)));


void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t new_size);
void free(void *ptr);

#endif