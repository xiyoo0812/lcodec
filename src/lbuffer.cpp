#define LUA_LIB

#include "lbuffer.h"

namespace lbuffer {

    static slice* new_slice(lua_State* L, std::string data) {
        return new slice((uint8_t*)data.c_str(), data.size());
    }
    
    static serializer* new_serializer(lua_State* L) {
        return new serializer();
    }

    luakit::lua_table open_lbuffer(lua_State* L) {
        luakit::kit_state kit_state(L);
        auto llbuffer = kit_state.new_table();
        llbuffer.set_function("new_slice", new_slice);
        llbuffer.set_function("new_serializer", new_serializer);
        kit_state.new_class<slice>(
            "size", &slice::size,
            "read", &slice::read,
            "check", &slice::check,
            "string", &slice::string,
            "contents", &slice::contents
            );
        kit_state.new_class<serializer>(
            "encode", &serializer::encode,
            "decode", &serializer::decode,
            "serialize", &serializer::serialize,
            "unserialize", &serializer::unserialize,
            "encode_string", &serializer::encode_string,
            "decode_string", &serializer::decode_string
            );
        return llbuffer;
    }
}

extern "C" {
    LUALIB_API int luaopen_lbuffer(lua_State* L) {
        auto lluabus = lbuffer::open_lbuffer(L);
        return lluabus.push_stack();
    }
}
