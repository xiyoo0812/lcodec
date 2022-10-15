#pragma once
#include "slice.h"

namespace lcodec {

    const size_t BUFFER_DEF = 64 * 1024;        //64K
    const size_t BUFFER_MAX = 16 * 1024 * 1024; //16M

    class var_buffer {
    public:
        var_buffer() { _alloc(); }
        ~var_buffer() { free(m_data); }

        void reset() {
            if (m_size != BUFFER_DEF) {
                m_data = (uint8_t*)realloc(m_data, BUFFER_DEF);
            }
            m_end = m_data + BUFFER_DEF;
            m_head = m_tail = m_data;
            m_size = BUFFER_DEF;
        }

        size_t size() {
            return m_tail - m_head;
        }

        size_t capacity() {
            return m_size;
        }

        size_t empty() {
            return m_tail == m_head;
        }

        size_t copy(size_t offset, const uint8_t* src, size_t src_len) {
            size_t data_len = m_tail - m_head;
            if (offset + src_len <= data_len) {
                memcpy(m_head + offset, src, src_len);
                return src_len;
            }
            return 0;
        }

        size_t push_data(const uint8_t* src, size_t push_len) {
            uint8_t* target = peek_space(push_len);
            if (target) {
                memcpy(target, src, push_len);
                m_tail += push_len;
                return push_len;
            }
            return 0;
        }

        size_t pop_data(uint8_t* dest, size_t pop_len) {
            size_t data_len = m_tail - m_head;
            if (pop_len > 0 && data_len >= pop_len) {
                memcpy(dest, m_head, pop_len);
                m_head += pop_len;
                return pop_len;
            }
            return 0;
        }

        size_t pop_size(size_t erase_len) {
            if (m_head + erase_len <= m_tail) {
                m_head += erase_len;
                size_t data_len = (size_t)(m_tail - m_head);
                if (m_size > BUFFER_DEF && data_len < m_size / 4) {
                    _regularize();
                    _resize(m_size / 2);
                }
                return erase_len;
            }
            return 0;
        }

        uint8_t* peek_data(size_t peek_len) {
            size_t data_len = m_tail - m_head;
            if (peek_len > 0 && data_len >= peek_len) {
                return m_head;
            }
            return nullptr;
        }

        size_t pop_space(size_t space_len) {
            if (m_tail + space_len <= m_end) {
                m_tail += space_len;
                return space_len;
            }
            return 0;
        }

        slice* get_slice(size_t len = 0, uint16_t offset = 0) {
            size_t data_len = m_tail - (m_head + offset);
            m_slice.attach(m_head + offset, len == 0 ? data_len : len);
            return &m_slice;
        }

        uint8_t* peek_space(size_t len) {
            size_t space_len = m_end - m_tail;
            if (space_len < len) {
                space_len = _regularize();
                if (space_len < len) {
                    size_t nsize = m_size * 2;
                    size_t data_len = m_tail - m_head;
                    while (nsize - data_len < len) {
                        nsize *= 2;
                    }
                    space_len = _resize(nsize);
                    if (space_len < len) {
                        return nullptr;
                    }
                }
            }
            return m_tail;
        }

        uint8_t* data(size_t* len) {
            *len = (size_t)(m_tail - m_head);
            return m_head;
        }

        std::string string() {
            size_t len = (size_t)(m_tail - m_head);
            return std::string((const char*)m_head, len);
        }

        template<typename T>
        void write(T value) {
            push_data((const uint8_t*)&value, sizeof(T));
        }

        void write(const char* src, size_t len) {
            push_data((const uint8_t*)src, len);
        }

    protected:
        //整理内存
        size_t _regularize() {
            size_t data_len = (size_t)(m_tail - m_head);
            if (m_head > m_data) {
                if (data_len > 0) {
                    memmove(m_data, m_head, data_len);
                }
                m_tail = m_data + data_len;
                m_head = m_data;
            }
            return m_size - data_len;
        }

        //重新设置长度
        size_t _resize(size_t size) {
            size_t data_len = (size_t)(m_tail - m_head);
            if (m_size == size || size < data_len || size > BUFFER_MAX) {
                return m_end - m_tail;
            }
            m_data = (uint8_t*)realloc(m_data, size);
            m_tail = m_data + data_len;
            m_end = m_data + size;
            m_head = m_data;
            m_size = size;
            return size - data_len;
        }

        void _alloc() {
            m_data = (uint8_t*)malloc(BUFFER_DEF);
            m_head = m_tail = m_data;
            m_end = m_data + BUFFER_DEF;
            m_size = BUFFER_DEF;
        }

    private:
        size_t m_size;
        uint8_t* m_head;
        uint8_t* m_tail;
        uint8_t* m_end;
        uint8_t* m_data;
        slice m_slice;
    };
}
