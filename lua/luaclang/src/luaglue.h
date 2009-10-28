#ifndef LUA_GLUE_H
#define LUA_GLUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#ifdef __cplusplus
}
#endif

/*
	Glue
	template annotation class to bind C++ types to Lua
	includes (single-)inheritance and many helper functions
*/
#pragma mark Glue
template <typename T>
class Glue {
public:
	// override this:
	static const char * usr_name();
	// and optionally these:
	static T * usr_new(lua_State * L);
	static T * usr_reinterpret(lua_State * L, int idx);
	static void usr_mt(lua_State * L);
	static void usr_lib(lua_State * L, const luaL_Reg * lib);
	static void usr_gc(lua_State * L, T * u);
	static int usr_tostring(lua_State * L, T * u);

	// utility methods:
	static void publish(lua_State * L, const char * superclass = NULL);
	static int push(lua_State * L, T * u);
	static T * to(lua_State * L, int idx = 1);
	static T * checkto(lua_State * L, int idx = 1);
	static int create(lua_State * L);
	static void erase(lua_State * L, int idx);

	// internal methods:
	static int gc(lua_State * L);
	static int tostring(lua_State * L);
	static const char * mt_name(lua_State * L);
};

/*
	Inline implementation
*/
#pragma mark Inline Implementation

template <typename T> T * Glue<T> :: usr_new(lua_State * L) { return 0; }
template <typename T> T * Glue<T> :: usr_reinterpret(lua_State * L, int idx) { return 0; } // if idx isn't a userdata
template <typename T> void Glue<T> :: usr_mt(lua_State * L) {}
template <typename T> void Glue<T> :: usr_lib(lua_State * L, const luaL_Reg * lib) {
	luaL_getmetatable(L, mt_name(L));
	while (lib->name) {
		lua_pushcclosure(L, lib->func, 0);
		lua_setfield(L, -2, lib->name);
		lib++;
	}
	lua_pop(L, 1);
}
template <typename T> void Glue<T> :: usr_gc(lua_State * L, T * u) {}
template <typename T> int Glue<T> :: usr_tostring(lua_State * L, T * u) { lua_pushfstring(L, "%s: %p", Glue<T>::usr_name(), u); return 1; }

template <typename T> int Glue<T> :: push(lua_State * L, T * u) {
	if (u==0)
		return luaL_error(L, "Cannot create null %s", usr_name());
	T ** udata = (T**)lua_newuserdata(L, sizeof(T*));
	luaL_getmetatable(L, mt_name(L));
	//lua::dump(L, usr_name());
	lua_setmetatable(L, -2);
	*udata = u;
	return 1;
}
template <typename T> int Glue<T> :: create(lua_State * L) {
	T * u = usr_new(L);
	return u ? push(L, u) : 0;
}
template <typename T> void Glue<T> :: erase(lua_State * L, int idx) {
	if (checkto(L, idx)) {
		T ** udata = (T **)lua_touserdata(L, idx);
		*udata = NULL;
	}
}
template <typename T> T * Glue<T> :: to(lua_State * L, int idx) {
	T * u = 0;
	lua_pushvalue(L, idx);
	if (lua_isuserdata(L, -1)) {
		if (lua_getmetatable(L, -1)) {
			lua_getfield(L, -1, Glue<T>::usr_name());
			if (!lua_isnoneornil(L, -1)) {
				u = *(T **)lua_touserdata(L, -3);
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	} else if (!lua_isnoneornil(L, idx)) {
		u = usr_reinterpret(L, lua_gettop(L));
	}
	lua_pop(L, 1);
	return u;
}
template <typename T> T * Glue<T> :: checkto(lua_State * L, int idx) {
	T * u = to(L, idx);
	if (u == 0) luaL_error(L, "%s not found (index %d)", usr_name(), idx);
	return u;
}
template <typename T> void Glue<T> :: publish(lua_State * L, const char * superclass) {
	luaL_newmetatable(L, mt_name(L));
	lua_pushstring(L, Glue<T>::usr_name());
	lua_pushboolean(L, true);
	lua_settable(L, -3);

	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	if (superclass != NULL) {
		const char * supername = lua_pushfstring(L, "meta_%s", superclass); lua_pop(L, 1);
		luaL_getmetatable(L, supername);
		lua_setmetatable(L, -2);
	}

	lua_pushcfunction(L, tostring);
	lua_setfield(L, -2, "__tostring");
	lua_pushcfunction(L, gc);
	lua_setfield(L, -2, "__gc");

	int mt = lua_gettop(L);
	usr_mt(L);
	lua_settop(L, mt);
}
template <typename T> int Glue<T> :: gc(lua_State * L) { T * u = to(L, 1); if (u) { Glue<T>::usr_gc(L, u); } return 0; }
template <typename T> int Glue<T> :: tostring(lua_State * L) {
	T * u = to(L, 1);
	if (u)
		Glue<T>::usr_tostring(L, u);
	else
		lua_pushfstring(L, "%s: nil", Glue<T>::usr_name());
	return 1;
}
template <typename T> const char * Glue<T> :: mt_name(lua_State * L) {
	const char * t = lua_pushfstring(L, "meta_%s", usr_name()); lua_pop(L, 1); return t;
}

#endif
