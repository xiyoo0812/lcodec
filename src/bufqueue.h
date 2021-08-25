#ifndef __BUF_QUEUE_H_
#define __BUF_QUEUE_H_

#include "lbuffer.h"

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
};

LBUFF_API struct bufqueue* queue_alloc(uint32_t fixsize);

LBUFF_API uint32_t queue_size(struct bufqueue* queue);

LBUFF_API uint32_t queue_empty(struct bufqueue* queue);

LBUFF_API void queue_clear(struct bufqueue* queue);

LBUFF_API void queue_close(struct bufqueue* queue);

LBUFF_API uint32_t queue_push(struct bufqueue* queue, const uint8_t* data, uint32_t sz);

LBUFF_API uint32_t queue_pop(struct bufqueue* queue, uint8_t* data, uint32_t sz);

LBUFF_API const uint8_t* queue_front(struct bufqueue* queue, uint32_t* len);

#ifdef __cplusplus
}
#endif

#endif