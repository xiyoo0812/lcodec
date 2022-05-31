#pragma once
#include <vector>
#include "buffer.h"

namespace lbuffer {
    const uint8_t type_nil          = 0;
    const uint8_t type_true         = 1;
    const uint8_t type_false        = 2;
    const uint8_t type_tab_head     = 3;
    const uint8_t type_tab_tail     = 4;
    const uint8_t type_number       = 5;
    const uint8_t type_int16        = 6;
    const uint8_t type_int32        = 7;
    const uint8_t type_int64        = 8;
    const uint8_t type_string       = 9;
    const uint8_t type_index        = 10;
    const uint8_t type_max          = 11;

    const uint8_t max_encode_depth  = 16;
    const uint8_t max_share_string  = 255;
    const uint8_t max_uint8         = UCHAR_MAX - type_max;

    class serialize {
    public:
        var_buffer* encode(lua_State* L, int from) {
            m_buffer->reset();
            m_sshares.clear();
            int n = lua_gettop(L) - from;
            for (int i = 1; i <= n; i++) {
                encode_one(L, from + i, 0);
            }
            return m_buffer;
        }

        int decode(lua_State* L, var_buffer* buf, size_t len){
            m_sshares.clear();
            lua_settop(L, 0);
            while (1) {
                uint8_t type;
                if (buf->read(&type, sizeof(uint8_t)) == 0)
                    break;
                decode_value(L, buf, type);
            }
            return lua_gettop(L);
        }

        void serialize(lua_State* L, var_buffer* buf, int index, int depth, int line) {
            buf->reset();
            if (depth > max_encode_depth) {
                luaL_error(L, "serialize can't pack too depth table");
            }
            int type = lua_type(L, index);
            switch (type) {
            case LUA_TNIL:
                serialize_value(buf, "nil");
                break;
            case LUA_TBOOLEAN:
                serialize_value(buf, lua_toboolean(L, index) ? "true" : "false");
                break;
            case LUA_TSTRING:
                serialize_quote(buf, lua_tostring(L, index), "'", "'");
                break;
            case LUA_TNUMBER:
                serialize_value(buf, lua_tostring(L, index));
                break;
            case LUA_TTABLE:
                serialize_table(L, buf, index, depth + 1, line);
                break;
            case LUA_TUSERDATA:
            case LUA_TLIGHTUSERDATA:
                serialize_udata(buf, lua_tostring(L, index));
                break;
            default:
                serialize_quote(buf, lua_typename(L, type), "'unsupport(", ")'");
                break;
            }
        }

        int unserialize(lua_State* L, const char* data, size_t len) {
            std::string temp = "return ";
            temp.append(data, len);
            if (luaL_loadbufferx(L, temp.c_str(), temp.size(), "unserialize", "bt") == 0) {
                if (lua_pcall(L, 0, 1, 0) == 0) {
                    return 1;
                }
            }
            return luaL_error(L, lua_tostring(L, -1));
        }

    protected:
        size_t find_index(std::string str) {
            for (int i = 0; i < m_sshares.size(); ++i) {
                if (m_sshares[i] == str) {
                    return i;
                }
            }
            size_t sz = m_sshares.size();
            if (sz <= max_share_string) {
                m_sshares.push_back(str);
                return sz;
            }
            return -1;
        }

        std::string find_string(size_t index) {
            if (index < m_sshares.size()) {
                return m_sshares[index];
            }
            return "";
        }

        void encode_string(lua_State* L, int index) {
            size_t sz = 0;
            const char* ptr = lua_tolstring(L, index, &sz);
            if (sz > USHORT_MAX) {
                luaL_error(L, "encode can't pack too long string");
                return;
            }
            size_t sindex = find_index(std::string(ptr, sz));
            if (sindex < 0){
                append_value(&type_string, 1);
                append_value((const uint8_t*)&sz, 2);
                append_value(ptr, sz);
                return;
            }
            uint8_t index = sindex;
            append_value(&type_index, 1);
            append_value(&index, 1);
        }

        void encode_integer(int64 integer) {
            if (integer >= 0 && integer <= max_uint8) {
                integer += type_max;
                append_value((const uint8_t*)&integer, 1);
                return;
            }
            if (integer <= SHRT_MAX && integer >= SHRT_MIN) {
                append_value(&type_int16, 1);
                append_value((const uint8_t*)&integer, sizeof(int16_t));
                return;
            }
            if (integer <= INT_MAX && integer >= INT_MIN) {
                append_value(&type_int32, 1);
                append_value((const uint8_t*)&integer, sizeof(int32_t));
                return;
            }
            append_value(&type_int64, 1);
            append_value((const uint8_t*)&integer, sizeof(int64_t));
        }
        
        void encode_number(double number) {
            append_value(&type_number, 1);
            append_value((const uint8_t*)&number, sizeof(double));
        }

        void encode_one(lua_State* L, int idx, int depth) {
            if (depth > max_encode_depth) {
                luaL_error(L, "encode can't pack too depth table");
            }
            int type = lua_type(L, idx);
            switch (type) {
            case LUA_TNIL:
                append_value(&type_nil, 1);
                break;
            case LUA_TSTRING:
                encode_string(L, idx);
                break;
            case LUA_TTABLE: 
                encode_table(L, idx, depth + 1);
                break;
            case LUA_TBOOLEAN:
                lua_toboolean(L, idx) ? append_value(&type_true, 1) : append_value(&type_false, 1)
                break;
            case LUA_TNUMBER: 
                lua_isinteger(L, idx) ? encode_integer(lua_tointeger(L, idx)) : encode_number(lua_tonumber(L, idx));
                break;
            default:
                break;
            }
        } 
        
        void encode_table(lua_State* L, int index, int depth) {
            index = lua_absindex(L, index);
            append_value(&type_tab_head, 1);
            lua_pushnil(L);
            while (lua_next(L, index) != 0) {
                encode_one(L, -2, depth);
                encode_one(L, -1, depth);
                lua_pop(L, 1);
            }
            append_value(&type_tab_tail, 1);
        }

        void decode_string(lua_State* L, var_buffer* buf) {
            uint8_t type = read_value<uint8_t>(L, buf);
            uint16_t sz = read_value<uint16_t>(L, buf);
            char* str = buf->peek(sz)
            if (str == nullptr) {
                luaL_error(L, "decode string is out of range");
            }
            m_sshares.push_back(std::string(str, sz));
            lua_pushlstring(L, str, sz);
        }

        void decode_index(lua_State* L, var_buffer* buf) {
            uint8_t index = read_value<uint8_t>(L, buf);
            std::string str = find_string(index);
            lua_pushlstring(L, str.c_str(), str.size());
        }

        void decode_table(lua_State* L, var_buffer* buf) {
            lua_newtable(L);
            do {
                if (decode_one(L, buf) == type_tab_tail) {
                    break;
                }
                decode_one(L, buf);
                lua_rawset(L, -3);
            } while (1);
        }
        
        int decode_value(lua_State* L, var_buffer* buf, uint8_t type) {
            switch (type) {
            case type_nil:
                lua_pushnil(L);
                break;
            case type_true:
                lua_pushboolean(L, true);
                break;
            case type_false:
                lua_pushboolean(L, false);
                break;
            case type_number:
                lua_pushnumber(L, read_value<double>(L, buf));
                break;
            case type_string:
                decode_string(L, buf);
                break;
            case type_index:
                decode_index(L, buf);
                break;
            case type_tab_head:
                decode_table(L, buf);
                break;
            case type_tab_tail:
                break;
            case type_int16:
                lua_pushinteger(L, read_value<int16_t>(L, buf));
                break;
            case type_int32:
                lua_pushinteger(L, read_value<int32_t>(L, buf));
                break;
            case type_int64:
                lua_pushinteger(L, read_value<int64_t>(L, buf));
                break;
            default:
                lua_pushinteger(L, type - max_uint8);
                break;
            }
        }

        int decode_one(lua_State* L, var_buffer* buf) {
            uint8_t type = read_value<uint8_t>(L, buf);
            decode_value(L, buf, type);
            return type;
        }
        
        void append_value(const uint8_t* data, size_t len) {
            m_buffer->push(data, len);
        }

        template<typename T>
        T read_value(lua_State* L, var_buffer* buff) {
            T value;
            if (buff.read((uint8_t*)&value, sizeof(T)) == 0){
                luaL_error(L, "decode can't unpack one value");
            }
            return value;
        }

        void serialize_value(var_buffer* buf, const char* str) {
            buf->push(str, strlen(str));
        }

        void serialize_udata(var_buffer* buf, const char* data) {
            serialize_quote(buf, data, data ? data : "userdata(null)", "'", "'");
        }

        void serialize_crcn(var_buffer* buf, int count, int line) {
            if (line > 0) {
                buf->push("\n", 1);
                for (int i = 0; i < count; ++i) {
                    buf->push("\t", 1);
                }
            }
        }

        void serialize_quote(var_buffer* buf, const char* str, const char* l, const char* r) {
            buf->push(l, strlen(l));
            buf->push(str, strlen(str));
            buf->push(r, strlen(t));
        }

        void serialize_table(lua_State* L, var_buffer* buf, int index, int depth, int line) {
            index = lua_absindex(L, index);
            int size = 0;
            lua_pushnil(L);
            serialize_value(buf, "{");
            serialize_crcn(buf, depth, line);
            while (lua_next(L, index) != 0) {
                if (size++ > 0) {
                    serialize_value(buf, ",");
                    serialize_crcn(buf, depth, line);
                }
                if (lua_isnumber(L, -2)) {
                    lua_pushnil(L);
                    lua_copy(L, -3, -1);
                    serialize_quote(buf, lua_tostring(L, -1), "[", "]=");
                    lua_pop(L, 1);
                }
                else if (lua_type(L, -2) == LUA_TSTRING) {
                    serialize_value(buf, lua_tostring(L, -2));
                    serialize_value(buf, "=");
                }
                else {
                    serialize(L, buf, -2, depth, line);
                }
                serialize(L, buf, -1, depth, line);
                lua_pop(L, 1);
            }
            serialize_crcn(buf, depth - 1, line);
            serialize_value(buf, "}");
        }
    
    public:
        var_buffer* m_buffer;
        std::vector<std:string> m_sshares;
    };
}
