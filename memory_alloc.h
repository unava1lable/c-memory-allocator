#ifndef _MEMORY_ALLOC_T
#define _MEMORY_ALLOC_T

#include <unistd.h>

struct header_t {
    size_t size;
    int is_free;
    struct header_t *next;
}__attribute__((aligned(16)));


void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t new_size);
void free(void *ptr);

void *aligned_alloc(size_t alignment, size_t size);

#endif