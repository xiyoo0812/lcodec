#define LUA_LIB

#include "lbuffer.h"

namespace lbuffer {

    static var_buffer* create_buffer(lua_State* L, int size) {
        return new var_buffer(size);
    }
    
    static serialize* create_serialize(lua_State* L) {
        return new serialize();
    }

    luakit::lua_table open_lbuffer(lua_State* L) {
        luakit::kit_state kit_state(L);
        auto llbuffer = kit_state.new_table();
        llbuffer.set_function("create_buffer", create_buffer);
        llbuffer.set_function("create_serialize", create_serialize);
        kit_state.new_class<var_buffer>(
            "copy", &var_buffer::copy,
            "push", &var_buffer::push,
            "size", &var_buffer::size,
            "check", &var_buffer::check,
            "reset", &var_buffer::reset,
            "slice", &var_buffer::slice,
            "contents", &var_buffer::contents,
            "pop_space", &var_buffer::pop_space
            );
        kit_state.new_class<serialize>(
            "encode", &serialize::encode,
            "decode", &serialize::decode,
            "serialize", &serialize::serialize,
            "unserialize", &serialize::unserialize
            );
        return llbuffer
    }
}

extern "C" {
    LUALIB_API int luaopen_lbuffer(lua_State* L) {
        auto lluabus = lbuffer::open_lbuffer(L);
        return lluabus.push_stack();
    }
}
