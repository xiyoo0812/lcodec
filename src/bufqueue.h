#ifndef __BUF_QUEUE_H_
#define __BUF_QUEUE_H_

#include "lbuffer.h"
#include "bufpool.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fixbuf {
    uint32_t end;
    uint32_t begin;
    uint32_t len;
    uint8_t* data;
    struct fixbuf* next;
};

struct bufqueue {
    uint32_t size;
    uint32_t fix_size;
    struct fixbuf* head;
    struct fixbuf* tail;
    struct bufpool* pool;
};

LBUFF_API struct bufqueue* bufqueue_alloc(uint32_t fix_size, uint16_t graw_size);

LBUFF_API uint32_t bufqueue_size(struct bufqueue* queue);

LBUFF_API uint32_t bufqueue_empty(struct bufqueue* queue);

LBUFF_API void bufqueue_clear(struct bufqueue* queue);

LBUFF_API void bufqueue_close(struct bufqueue* queue);

LBUFF_API uint32_t bufqueue_push(struct bufqueue* queue, const uint8_t* data, uint32_t sz);

LBUFF_API uint32_t bufqueue_pop(struct bufqueue* queue, uint8_t* data, uint32_t sz);

LBUFF_API const uint8_t* bufqueue_front(struct bufqueue* queue, uint32_t* len);

#ifdef __cplusplus
}
#endif

#endif