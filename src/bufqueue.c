#include "bufqueue.h"
#include <memory.h>

struct fixbuf* fixbuf_alloc(uint32_t size) {
    struct fixbuf* fb = (struct fixbuf*)malloc(sizeof(struct fixbuf));
    fb->data = (uint8_t*)malloc(size);
    fb->begin = fb->end = 0;
    fb->len = 0;
    fb->next = NULL;
    return fb;
}

void fixbuf_close(struct fixbuf* head) {
    while (head) {
        struct fixbuf* next = head->next;
        if (head->data) {
            free(head->data);
            head->data = NULL;
        }
        free(head);
        head = next;
    }
}

struct bufqueue* queue_alloc(uint32_t fixsize) {
    struct bufqueue* queue = (struct bufqueue*)malloc(sizeof(struct bufqueue));
    queue->head = queue->tail = NULL;
    queue->fix_size = fixsize;
    queue->size = 0;
    return queue;
}

uint32_t queue_size(struct bufqueue* queue) {
    return queue->size;
}

uint32_t queue_empty(struct bufqueue* queue) {
    return queue->size == 0;
}

uint32_t queue_full(struct bufqueue* queue) {
    return queue->tail->end == queue->tail->len;
}

void queue_clear(struct bufqueue* queue) {
    fixbuf_close(queue->head);
    queue->head = queue->tail = NULL;
    queue->size = 0;
}

void queue_close(struct bufqueue* queue) {
    queue_clear(queue);
    free(queue);
}

uint32_t queue_push(struct bufqueue* queue, const uint8_t* data, uint32_t sz) {
    uint32_t push_len = 0;
    while (push_len < sz) {
        if (queue_empty(queue) || queue_full(queue)) {
            struct fixbuf* fb = fixbuf_alloc(queue->fix_size);
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

uint32_t queue_pop(struct bufqueue* queue, uint8_t* data, uint32_t sz) {
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
                fixbuf_close(head);
            }
            pop_len += cpylen;
        }
        queue->size -= sz;
        return sz;
    }
    return 0;
}

const uint8_t* queue_front(struct bufqueue* queue, uint32_t* len) {
    if (!queue->head) {
        return NULL;
    }
    struct fixbuf* head = queue->head;
    *len = head->end - head->begin;
    return head->data + head->begin;
}

