/*
	Lua wrapper to LLVM-Clang
 */

#ifndef INC_LUA_CLANG_H
#define INC_LUA_CLANG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luaopen_clang(lua_State * L);

#ifdef __cplusplus
}
#endif

#endif
