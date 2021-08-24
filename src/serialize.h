#ifndef __SERIALIZE_H__
#define __SERIALIZE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>

void encode(lua_State* L, struct buffer* buf, int from);
void decode(lua_State* L, struct buffer* buf);

void serialize(lua_State* L, struct buffer* buf, int index, int depth);

#ifdef __cplusplus
}
#endif

#endif