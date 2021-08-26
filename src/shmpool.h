#ifndef __SHM_POOL_H_
#define __SHM_POOL_H_

#include "lbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct shmblock {
    uint8_t* data;
    struct shmblock* next;
    struct shmblock* next_free;
};

struct shmpool {
    uint32_t used;
    uint32_t fix_size;
    uint16_t shm_size;
    uint8_t* shm_data;
    size_t shm_handle;
    struct shmblock* head;
    struct shmblock* tail;
    struct shmblock* first_free;
};

//shm
LBUFF_API uint8_t* attach_shm(size_t shm_id, size_t size, size_t* shm_handle);

LBUFF_API void detach_shm(uint8_t* shm_buff, size_t shm_handle);

LBUFF_API void delete_shm(size_t shm_handle);

//shmpool
LBUFF_API struct shmpool* shmpool_alloc(uint32_t fixsize, uint16_t shm_size, size_t shm_id);

LBUFF_API void shmpool_close(struct shmpool* pool);

LBUFF_API uint8_t* shmpool_malloc(struct shmpool* pool);

LBUFF_API void shmpool_free(struct shmpool* pool, uint8_t* data);


#ifdef __cplusplus
}
#endif

#endif