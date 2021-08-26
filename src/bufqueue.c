#include "bufqueue.h"
#include <memory.h>

struct fixbuf* fixbuf_alloc(struct bufpool* pool) {
    struct fixbuf* fb = (struct fixbuf*)malloc(sizeof(struct fixbuf));
    fb->data = bufpool_malloc(pool);
    fb->begin = fb->end = 0;
    fb->len = 0;
    fb->next = NULL;
    return fb;
}

void fixbuf_close(struct bufpool* pool, struct fixbuf* head) {
    while (head) {
        struct fixbuf* next = head->next;
        if (head->data) {
            bufpool_free(pool, head->data);
            head->data = NULL;
        }
        free(head);
        head = next;
    }
}

struct bufqueue* bufqueue_alloc(uint32_t fix_size, uint16_t graw_size) {
    struct bufqueue* queue = (struct bufqueue*)malloc(sizeof(struct bufqueue));
    queue->pool = bufpool_alloc(fix_size, graw_size);
    queue->head = queue->tail = NULL;
    queue->fix_size = fix_size;
    queue->size = 0;
    return queue;
}

uint32_t bufqueue_size(struct bufqueue* queue) {
    return queue->size;
}

uint32_t bufqueue_empty(struct bufqueue* queue) {
    return queue->size == 0;
}

uint32_t queue_full(struct bufqueue* queue) {
    return queue->tail->end == queue->tail->len;
}

void bufqueue_clear(struct bufqueue* queue) {
    fixbuf_close(queue->pool, queue->head);
    queue->head = queue->tail = NULL;
    queue->size = 0;
}

void bufqueue_close(struct bufqueue* queue) {
    bufqueue_clear(queue);
    bufpool_close(queue->pool);
    free(queue);
}

uint32_t bufqueue_push(struct bufqueue* queue, const uint8_t* data, uint32_t sz) {
    uint32_t push_len = 0;
    while (push_len < sz) {
        if (bufqueue_empty(queue) || queue_full(queue)) {
            struct fixbuf* fb = fixbuf_alloc(queue->pool);
            if (!queue->head) {
                queue->head = fb;
            }
            if (queue->tail) {
                queue->tail->next = fb;
            }
            queue->tail = fb;
        }
        struct fixbuf* tail = queue->tail;
        long cpylen = (sz - push_len) < (tail->len - tail->end) ? (sz - push_len) : (tail->len - tail->end);
        memcpy(tail->data + tail->end, data + push_len, cpylen);
        tail->end += cpylen;
        push_len += cpylen;
    }
    queue->size += sz;
    return sz;
}

uint32_t bufqueue_pop(struct bufqueue* queue, uint8_t* data, uint32_t sz) {
    if (sz > 0 && sz <= queue->size) {
        uint32_t pop_len = 0;
        while (pop_len < sz) {
            struct fixbuf* head = queue->head;
            uint32_t cpylen = (sz - pop_len) < (head->end - head->begin) ? (sz - pop_len) : (head->end - head->begin);
            if (data) {
                memcpy(data + pop_len, head->data + head->begin, cpylen);
            }
            head->begin += cpylen;
            if (head->begin == head->end) {
                queue->head = head->next;
                head->next = NULL;
                fixbuf_close(queue->pool, head);
            }
            pop_len += cpylen;
        }
        queue->size -= sz;
        return sz;
    }
    return 0;
}

const uint8_t* bufqueue_front(struct bufqueue* queue, uint32_t* len) {
    if (!queue->head) {
        return NULL;
    }
    struct fixbuf* head = queue->head;
    *len = head->end - head->begin;
    return head->data + head->begin;
}

