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
    block = sbrk(total_size); // todo: align to 16-byte or page-size
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
    header = (struct header_t *)program_break - 1;

    program_break = sbrk(0);

    // at the top of heap
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