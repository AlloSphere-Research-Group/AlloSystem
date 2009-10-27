

#include "delta_lua.h"

#include "al_time.h"
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
		local f, err = loadfile(path .. '%s'); \
		if f then delta.go(f) else print(err) end ", path, file);
	if (luaL_dostring(L, gocode)) {
		printf("%s\n", lua_tostring(L, -1));
		return;
	}
}

int main(int ac, char * av) {
	
	delta_main_init();

	lua_State * L = lua_open();
	luaL_openlibs(L);
	script_preload(L, "audio", luaopen_audio);
	script_preload(L, "delta", luaopen_delta);
	script_run(L, "test_delta.lua");
	
	while (delta_main_now() < 3) {
		delta_main_tick();
		printf(" %5.2f\n", delta_main_now());
		
		// simulate audio thread (non-realtime)
		delta_audio_tick(441);
		
		// simulate realtime
		//al_sleep(0.01);
	}
	
	lua_close(L);
	delta_main_quit();
	
	return 0;
}