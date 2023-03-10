#ifndef _MEMORY_ALLOC_T
#define _MEMORY_ALLOC_T

#include <unistd.h>

#pragma pack(16)
struct header_t {
    size_t size;
    int is_free;
    struct header_t *next;
};
#pragma pack()

void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t new_size);
void free(void *ptr);

void *aligned_alloc(size_t alignment, size_t size);

#endif