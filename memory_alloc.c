#include "memory_alloc.h"
#include <pthread.h>
#include <unistd.h>

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
        return (void *)(header + 1); // escape header part
    }

    total_size = size + sizeof(struct header_t);
    block = sbrk(total_size); // todo: align to 16-byte
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

int main(void) {
    printf("sizeof header is %zd\n", sizeof(struct header_t));
    void *ptr = sbrk(0);
    printf("program break is %p\n", ptr);
    malloc(16);
    void *new_ptr = sbrk(0);
    printf("program break is %p\n", new_ptr);
    printf("memory size: %ld\n", (long)new_ptr - (long)ptr);
}