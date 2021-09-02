#ifndef __BUF_QUEUE_H_
#define __BUF_QUEUE_H_

#include "lbuffer.h"
#include "bufpool.h"
#include "shmpool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fixbuffer {
    uint32_t len;
    uint32_t end;
    uint32_t begin;
    uint8_t* data;
    struct fixbuffer* next;
} fix_buffer;

typedef struct bufqueue {
    uint32_t size;
    uint32_t fix_size;
    fix_buffer* head;
    fix_buffer* tail;
    shm_pool* sh_pool;
    buffer_pool* buf_pool;
} bufder_queue;

LBUFF_API bufder_queue* bufqueue_alloc(uint32_t fix_size);

LBUFF_API void bufqueue_pool(bufder_queue* queue, uint16_t graw_size);

LBUFF_API void bufqueue_shm(bufder_queue* queue, uint16_t block_num, size_t shm_id);

LBUFF_API uint32_t bufqueue_size(bufder_queue* queue);

LBUFF_API uint32_t bufqueue_empty(bufder_queue* queue);

LBUFF_API void bufqueue_clear(bufder_queue* queue);

LBUFF_API void bufqueue_close(bufder_queue* queue);

LBUFF_API uint32_t bufqueue_push(bufder_queue* queue, const uint8_t* data, uint32_t sz);

LBUFF_API uint32_t bufqueue_pop(bufder_queue* queue, uint8_t* data, uint32_t sz);

LBUFF_API const uint8_t* bufqueue_front(bufder_queue* queue, uint32_t* len);

#ifdef __cplusplus
}
#endif

#endif