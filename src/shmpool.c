#include "shmpool.h"

#ifdef WIN32
#include <windows.h>
uint8_t* attach_shm(size_t shm_id, size_t size, size_t* shm_handle) {
    char name_buff[128];
    snprintf(name_buff, sizeof(name_buff), "shm_%zu", shm_id);
    HANDLE fileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name_buff);
    if (!fileMapping) {
        fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, size, name_buff);
        if (!fileMapping) {
            return NULL;
        }
    }
    uint8_t* shm_buff = (uint8_t*)MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    *shm_handle = (size_t)fileMapping;
    return shm_buff;
}

void detach_shm(uint8_t* shm_buff, size_t shm_handle) {
    UnmapViewOfFile(shm_buff);
    HANDLE fileMapping = (HANDLE)shm_handle;
    if (fileMapping) {
        CloseHandle(fileMapping);
    }
}

void delete_shm(size_t shm_handle) {
}

#else
#include <sys/ipc.h>
#include <sys/shm.h>

uint8_t* attach_shm(size_t shm_id, size_t size, size_t* shm_handle) {
    int handle = shmget(shm_id, 0, 0);
    if (handle < 0) {
        handle = shmget(shm_id, size, 0666 | IPC_CREAT);
        if (handle < 0) {
            return NULL;
        }
    }
    uint8_t* shm_buff = shmat(handle, 0, 0);
    if (shm_buff == (uint8_t*)-1) {
        return NULL;
    }
    *shm_handle = handle;
    return shm_buff;
}

void detach_shm(uint8_t* shm_buff, size_t shm_handle) {
    shmdt(shm_buff);
}

void delete_shm(size_t shm_handle) {
    if (shm_handle > 0) {
        shmctl(shm_handle, IPC_RMID, NULL);
    }
}
#endif

struct shmblock* shmblock_alloc(uint8_t* data, uint32_t* offset, uint32_t size) {
    struct shmblock* fb = (struct shmblock*)(data + *offset);
    *offset += sizeof(struct shmblock);
    fb->data = (uint8_t*)(data + *offset);
    fb->next_free = NULL;
    fb->next = NULL;
    *offset += size;
    return fb;
}

struct shmpool* shmpool_alloc(uint32_t fixsize, uint16_t shm_size, size_t shm_id) {
    size_t handle = 0, offset = 0;
    uint8_t* shm_data = attach_shm(shm_id, shm_size, &handle);
    if (!shm_data) {
        return NULL;
    }
    struct shmpool* pool = (struct shmpool*)shm_data;
    pool->shm_data = shm_data;
    pool->shm_handle = handle;
    if (pool->used) {
        return pool;
    }
    pool->head = pool->tail = pool->first_free = NULL;
    pool->shm_size = shm_size;
    pool->fix_size = fixsize;
    pool->used = 0;
    return pool;
}

void shmpool_close(struct shmpool* pool){
    detach_shm(pool->shm_data, pool->shm_handle);
    delete_shm(pool->shm_handle);
}
