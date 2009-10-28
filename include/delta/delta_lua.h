#ifndef INC_DELTA_LUA_H
#define INC_DELTA_LUA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "delta.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luaopen_delta(lua_State * L);
extern int luaopen_audio(lua_State * L);

/* Lua/Audio API */
#define DELTA_INSTANCE_LITERAL "__delta_instance"
#define AUDIO_PROCESS_LITERAL "__delta_audio_process"
#define AUDIO_BUS_LITERAL "__delta_audio_bus"

/* simplifies lua_to* lua_push* lua_opt* in codegen */
typedef double number;

#define lua_todouble(L, idx) lua_tonumber(L, idx)
#define luaL_checkdouble(L, idx) luaL_checknumber(L, idx)
#define luaL_optdouble(L, idx, def) luaL_optnumber(L, idx, def)

#define lua_tosample(L, idx) lua_tonumber(L, idx)
#define luaL_checksample(L, idx) luaL_checknumber(L, idx)
#define luaL_optsample(L, idx, def) luaL_optnumber(L, idx, def)

//extern param luaL_optparam(lua_State * L, int idx, param def);
//extern param lua_toparam(lua_State * L, int idx);

extern process lua_toprocess(lua_State * L, int idx);
extern int lua_audio_gc(lua_State * L);
extern int lua_audio_newindex(lua_State * L);
extern int lua_audio_index(lua_State * L);

extern bus lua_tobus(lua_State * L, int idx);
extern bus luaL_checkbus(lua_State * L, int idx);
extern bus luaL_optbus(lua_State * L, int idx, bus def);
extern bus lua_bus_push(lua_State * L);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INC_DELTA_LUA_H */