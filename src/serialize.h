#pragma once
#include <unorder_map>

#include "buffer.h"

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
}


namespace lbuffer {
    class serialize {
    public:
        var_buffer* encode(lua_State* L, int depth) {
            return m_buffer;
        }

        int decode(lua_State* L, var_buffer* buff, size_t len){

        }
    
    public:
        var_buffer* m_buffer;
        std::unorder_map<uint16_t, std:string> m_share_str;
    };
}
