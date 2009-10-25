#include "delta_lua.h"
#include "unistd.h"
#include "stdio.h"


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
	sprintf(gocode, "require 'delta' \
		path = '%s/' \
		package.path = path .. '?.lua;' .. package.path \
		for k, v in pairs(delta) do _G[k] = v end \
		local f, err = loadfile('%s/%s'); \
		if f then go(f) else print(err) end ", path, path, file);
	if (luaL_dostring(L, gocode)) {
		printf("%s\n", lua_tostring(L, -1));
		return;
	}
}

int main(int ac, char * av) {

	lua_State * L = lua_open();
	luaL_openlibs(L);
	
	//script_preload(L, "audio", luaopen_audio);
	script_preload(L, "delta", luaopen_delta);
	
	script_run(L, "test_delta.lua");
	
	
	return 0;
}