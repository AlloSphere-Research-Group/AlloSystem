
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"

#include "unistd.h"

void script_preload(lua_State * L, const char * name, lua_CFunction func) {
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, name);
	lua_settop(L, 0);
}

void script_run(lua_State * L, char * file) {
	char path[256];
	getcwd(path, 255);
	char gocode [1024];
	sprintf(gocode, "path = '%s/' \
		package.path = path .. '?.lua;' .. package.path \
		package.cpath = path .. '?.so;' .. package.cpath \
		local f, err = loadfile(path .. '%s'); \
		if f then f() else print(err) end ", path, file);
	if (luaL_dostring(L, gocode)) {
		printf("%s\n", lua_tostring(L, -1));
		return;
	}
}

int main (int ac, char ** av) {
	
	lua_State * L = lua_open();
	luaL_openlibs(L);
	
	script_run(L, "test_jit.lua");

	lua_close(L);
	
	return 0;
}