// unl_shm.h
#ifndef __SHM_H_
#define __SHM_H_

#include "lbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

LBUFF_API uint8_t* find_shm(size_t shm_id, size_t* shm_handle);

LBUFF_API uint8_t* attach_shm(size_t shm_id, size_t size, size_t* shm_handle);

LBUFF_API void detach_shm(uint8_t* shm_buff, size_t shm_handle);

LBUFF_API void delete_shm(size_t shm_handle);


#ifdef __cplusplus
}
#endif

#endif