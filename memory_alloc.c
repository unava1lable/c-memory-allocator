#define _DEFAULT_SOURCE // to use MAP_ANONYMOUS flag
#include "memory_alloc.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

// if size >= LIMIT, use mmap() to allocate memory
// if size < LIMIT, use sbrk() to allocate memory
#define LIMIT 128 * 1024

static struct header_t *head, *tail;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// get block having enough size from free block list
static struct header_t *get_free_block(size_t size) {
    struct header_t *cur = head;
    while (cur != NULL) {
        if (cur->is_free && cur->size >= size) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

static inline void *sbrk_alloc(size_t size) {
    void *block = sbrk(size);
    return block;
}

static void *mmap_alloc(size_t size) {
    void *block = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED) {
        return (void *)-1;
    }
    return block;
}

static void *memset(void *str, int c, size_t n) {
    unsigned char uc = c;
    unsigned char *c_str;
    
    for (c_str = str; n > 0; c_str++, n--) {
        *c_str = uc;
    }

    return str;
}

static void *memcpy(void *dest, const void *src, size_t size) {
    if (dest == NULL || src == NULL) {
        return NULL;
    }
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while (size--) {
        *d++ = *s++;
    }
    return dest;
}

void *malloc(size_t size) {
    size_t total_size;
    void *block;
    struct header_t *header;

    if (!size) {
        return NULL;
    }
    pthread_mutex_lock(&mtx);
    header = get_free_block(size);

    if (header) {
        header->is_free = 0;
        pthread_mutex_unlock(&mtx);
        return (void *)(header + 1); // skip header block
    }

    // align to pagesize (4096-byte)
    size = (size / 4096) * 4096 + ((size % 4096 == 0) ? 0 : 4096);
    total_size = size + sizeof(struct header_t);
    block = sbrk(total_size);
    if (block == (void *)-1) {
        pthread_mutex_unlock(&mtx);
        return NULL;
    }
    header = block;
    header->size = size;
    header->is_free = 0;
    header->next = NULL;

    if (head == NULL) {
        head = header;
    }

    if (tail) {
        tail->next = header;
    }
    tail = header;
    pthread_mutex_unlock(&mtx);
    return (void *)(header + 1);
}

void free(void *ptr) {
    struct header_t *header, *temp;
    void *program_break;

    if (ptr == NULL) {
        return;
    }
    pthread_mutex_lock(&mtx);
    
    // get header block
    header = (struct header_t *)ptr - 1;

    program_break = sbrk(0);

    // if the block is at the top of heap, release it
    if ((char *)ptr + header->size == program_break) {
        if (head == tail) {
            head = tail = NULL;
        } else {
            temp = head;
            while (temp != NULL) {
                if (temp->next == tail) {
                    temp->next = NULL;
                    tail = temp;
                }
                temp = temp->next;
            }
        }
        sbrk(0 - sizeof(struct header_t) - header->size);
        pthread_mutex_unlock(&mtx);
        return;
    }
    header->is_free = 1;
    pthread_mutex_unlock(&mtx);
}

void *calloc(size_t num, size_t size) {
    if (num == 0 || size == 0) {
        return NULL;
    }

    size_t total_size = num * size;
    void *block;

    block = malloc(total_size);
    if (block == NULL) {
        return block;
    }
    memset(block, 0, size);
    return block;
}

void *realloc(void *ptr, size_t new_size) {
    struct header_t *header;
    void *ret;

    if (ptr == NULL || new_size == 0) {
        return malloc(new_size);
    }
    header = (struct header_t *)ptr - 1;
    if (header->size >= new_size) {
        return ptr;
    }
    ret = malloc(new_size);
    if (ret != NULL) {
        memcpy(ret, ptr, header->size);
        free(ptr);
    }

    return ret;
}