#ifndef __MEMORY_H_
#define __MEMORY_H_

#include "lbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fixblock {
    uint8_t* data;
    struct fixblock* next;
    struct fixblock* next_free;
};

struct bufpool {
    uint32_t used;
    uint32_t fix_size;
    uint16_t graw_size;
    struct fixblock* head;
    struct fixblock* tail;
    struct fixblock* first_free;
};

LBUFF_API struct bufpool* pool_alloc(uint32_t fixsize, uint16_t graw_size);

LBUFF_API uint32_t pool_used(struct bufpool* pool);

LBUFF_API void pool_close(struct bufpool* pool);

LBUFF_API uint8_t* pool_malloc(struct bufpool* pool);

LBUFF_API void pool_free(struct bufpool* pool, uint8_t* data);


#ifdef __cplusplus
}
#endif

#endif