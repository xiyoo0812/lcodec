#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdint.h>
#include "lbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct buffer {
    uint8_t* head;
    uint8_t* tail;
    uint8_t* end;
    uint8_t* data;
    size_t ori_size;
    size_t size;
};

//分配buffer
LBUFF_API struct buffer* buffer_alloc(size_t size);
//释放buffer
LBUFF_API void buffer_close(struct buffer* buf);
//重置
LBUFF_API void buffer_reset(struct buffer* buf);
//获取buffsize
LBUFF_API size_t buffer_size(struct buffer* buf);
//复制
LBUFF_API size_t buffer_copy(struct buffer* buf, size_t offset, const uint8_t* src, size_t src_len);
//写入
LBUFF_API size_t buffer_apend(struct buffer* buf, const uint8_t* src, size_t src_len);
//移动头指针
LBUFF_API size_t buffer_erase(struct buffer* buf, size_t erase_len);
//全部数据
LBUFF_API uint8_t* buffer_data(struct buffer* buf, size_t* len);
//尝试读出
LBUFF_API size_t buffer_peek(struct buffer* buf, uint8_t* dest, size_t peek_len);
//读出
LBUFF_API size_t buffer_read(struct buffer* buf, uint8_t* dest, size_t read_len);
//返回可写指针
LBUFF_API uint8_t* buffer_attach(struct buffer* buf, size_t len);
//移动尾指针
LBUFF_API size_t buffer_grow(struct buffer* buf, size_t graw_len);

#ifdef __cplusplus
}
#endif

#endif
