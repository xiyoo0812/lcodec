#include "bufpool.h"

struct fixblock* fixblock_alloc(uint32_t size) {
    struct fixblock* fb = (struct fixblock*)malloc(sizeof(struct fixblock));
    fb->data = (uint8_t*)malloc(size);
    fb->next_free = NULL;
    fb->next = NULL;
    return fb;
}

void fixblock_close(struct fixblock* head) {
    while (head) {
        struct fixblock* next = head->next;
        if (head->data) {
            free(head->data);
            head->data = NULL;
        }
        free(head);
        head = next;
    }
}

struct bufpool* pool_alloc(uint32_t fixsize, uint16_t graw_size) {
    struct bufpool* pool = (struct bufpool*)malloc(sizeof(struct bufpool));
    pool->head = pool->tail = pool->first_free = NULL;
    pool->graw_size = graw_size;
    pool->fix_size = fixsize;
    pool->used = 0;
    return pool;
}

uint32_t pool_used(struct bufpool* pool) {
    return pool->used;
}

void pool_close(struct bufpool* pool) {
    fixblock_close(pool->head);
    pool->head = pool->tail = pool->first_free = NULL;
    pool->used = 0;
    free(pool);
}

uint8_t* pool_malloc(struct bufpool* pool) {
    if (!pool->first_free) {
        for (uint16_t i = 0; i < pool->graw_size; ++i) {
            struct fixblock* fb = fixblock_alloc(pool->fix_size);
            if (!pool->head) {
                pool->head = fb;
            }
            if (pool->tail) {
                pool->tail->next = fb;
            }
            if (!pool->first_free) {
                pool->first_free = fb;
            }
            pool->tail = fb;
        }
        struct fixblock* nfb = pool->first_free;
        while (nfb) {
            nfb->next_free = nfb->next;
            nfb = nfb->next;
        }
    }
    struct fixblock* block = pool->first_free;
    pool->first_free = block->next;
    pool->used++;
    return block->data;
}

void pool_free(struct bufpool* pool, uint8_t* data){
    struct fixblock* block = (struct fixblock*)data;
    block->next_free = pool->first_free;
    pool->first_free = block;
    pool->used--;
}
