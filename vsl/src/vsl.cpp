#include "vsl/vsl.h"

#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "stdio.h"

struct vsl_struct {
	lua_State * L;

};


static void script_loadlib(lua_State * L, const char * name, lua_CFunction func) {
	lua_pushcfunction(L, func);
	lua_pushstring(L, name);
	if (lua_pcall(L, 1, LUA_MULTRET, 0)) {
		printf("%s", lua_tostring(L, -1));
	}
	lua_settop(L, 0);
}

static void script_preloadlib(lua_State * L, const char * name, lua_CFunction func) {
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, name);
	lua_settop(L, 0);
}

static void state_init(lua_State * L, const char * path) {
	
	script_loadlib(L, LUA_COLIBNAME, luaopen_base);
	script_loadlib(L, LUA_MATHLIBNAME, luaopen_math);
	script_loadlib(L, LUA_STRLIBNAME, luaopen_string);
	script_loadlib(L, LUA_TABLIBNAME, luaopen_table);
	script_loadlib(L, LUA_IOLIBNAME, luaopen_io);
	script_loadlib(L, LUA_OSLIBNAME, luaopen_os);
	script_loadlib(L, LUA_LOADLIBNAME, luaopen_package);
	script_preloadlib(L, LUA_DBLIBNAME, luaopen_debug);
	script_preloadlib(L, LUA_BITLIBNAME, luaopen_bit);
	script_preloadlib(L, LUA_JITLIBNAME, luaopen_jit);

	if (path) {
		// add path to search paths, and cache in global:
		char initcode [1024];
		sprintf(initcode, "path = '%s/' \
			package.path = path .. '?.lua;' .. package.path \
			package.cpath = path .. '?.lua;' .. package.cpath ", path);
		if (luaL_dostring(L, initcode)) {
			printf("%s\n", lua_tostring(L, -1));
			return;
		}
	}

}

vsl vsl_create(const char * path) {
	vsl v = (vsl)malloc(sizeof(struct vsl_struct));
	if (v) {
		v->L = lua_open();
		state_init(v->L, path);
	}
	return v;
}

void vsl_destroy(vsl * vptr) {
	vsl v;
	if (vptr && *vptr) {
		v = *vptr;
		lua_close(v->L);
		free(v);
		*vptr = 0;
	}
}

lua_State * vsl_lua(vsl v) {
	return v ? v->L : NULL;
}