#include "delta_lua.h"

static delta_main audiomain = NULL;

//param luaL_optparam(lua_State * L, int idx, param def) {
//	double x, a;
//	if (lua_istable(L, idx)) {
//		lua_rawgeti(L, idx, 1); x = luaL_optnumber(L, -1, def.actual); lua_pop(L, 1);
//		lua_rawgeti(L, idx, 2); a = luaL_optnumber(L, -1, def.alpha); lua_pop(L, 1);
//	} else if (lua_isnumber(L, idx)) {
//		x = luaL_optnumber(L, idx, def.actual);
//		a = 0.95;
//	}
//	param p = { def.actual, x, a };
//	//printf("%f %f %f\n", def.actual, x, def.alpha);
//	return p;
//}
//
//param lua_toparam(lua_State * L, int idx) {
//	param def = { 0, 0, 0.95 };
//	return luaL_optparam(L, idx, def);
//}

process lua_toprocess(lua_State * L, int idx) {
	process p = NULL;
	lua_getmetatable(L, idx);
	lua_getfield(L, -1, AUDIO_PROCESS_LITERAL);	
	if (lua_toboolean(L, -1)) {
		p = *(process *)lua_touserdata(L, idx);
	}
	lua_pop(L, 2);
	return p;	
}

int lua_audio_gc(lua_State * L) {
	process self = lua_toprocess(L, 1);
	if (self) {
		if (delta_audio_proc_gc(&self)) {
			return luaL_error(L, "failed to send gc event for @name %p", self);
		}	
		/* trash the env */
		lua_pushnil(L);
		lua_setfenv(L, 1);
		
		/* clear metatable on object: */
		lua_pushnil(L);
		lua_setmetatable(L, -2);
	}
	return 0;
}

int lua_audio_newindex(lua_State * L) {
	/* stack: ugen, key, value */
	/* check if key exists in metatable inputs */
	lua_getfield(L, LUA_ENVIRONINDEX, "inputs"); // inputs table
	lua_pushvalue(L, 2); 			// key
	lua_gettable(L, -2);			
	if (!lua_isnoneornil(L, -1)) {
		lua_pop(L, 2); // inputs table, duff value
		
		/*
			need to create a new wire object
			when either input OR output are destroyed, wire should be too.
			
			relying on __gc doesn't work, because the userdata gc is called first.
			but: we only care if the receiver is destroyed (update the sender) 
			if sender is destroyed, receiver will stop receiving, but can still function
			so the update is actually on the sender, not the receiver!
			
			sender.readers.push(receiver)
			
			if (lua) receiver is destroyed, it needs to notify the (dsp) sender accordingly
				(before the receiver is deleted)
			if the (lua) sender is destroyed, we don't care 
				(except that the (lua)wire object now has a dangling pointer...)
					d
			can a weak table solve this?
			receiver can connect to multiple senders, or only one?
		*/
		
		
//		lua_getfenv(L, 1);				// userdata environment
//		lua_insert(L, 2);
//		lua_pushvalue(L, 1); 			// dest (should be the inlet by key)
//		lua_wire_create(L);
//		lua_rawset(L, -3);
		
		//  todo: if v is a Ugen, get output[1] 
		lua_getfenv(L, 1);				// userdata environment
		lua_pushvalue(L, 2);			// key
		lua_pushvalue(L, 3);			// value
		lua_rawset(L, -3);
		
	} else {
		printf("key %s not found\n", lua_tostring(L, 2));
	}
	return 0;
}

int lua_audio_index(lua_State * L) {
	lua_getmetatable(L, 1);
	lua_pushvalue(L, 2); //key
	lua_rawget(L, -2); // mt
	if (lua_isnoneornil(L, -1)) {
		lua_getfenv(L, 1);
		lua_pushvalue(L, 2); //key
		lua_rawget(L, -2); // fenv
	}
	return 1;
}

bus lua_tobus(lua_State * L, int idx) {
	bus * u = (bus *)luaL_checkudata(L, idx, "meta_bus");
	return u ? *u : NULL;
}
bus luaL_checkbus(lua_State * L, int idx) {
	bus self = lua_tobus(L, idx);
	if (self == NULL) {
		luaL_error(L, "error: bus expected (argument %d)", idx);
		return NULL;
	}
	return self;
}

bus luaL_optbus(lua_State * L, int idx, bus def) {
	bus b = def;
	if (lua_isuserdata(L, idx)) {
		lua_pushvalue(L, idx);
		lua_getmetatable(L, -1);
		luaL_getmetatable(L, "meta_bus");
		if (lua_rawequal(L, -1, -2)) {
			b = *(bus *)lua_touserdata(L, -3);
		} 
		lua_pop(L, 3);
	}
	return b;
}	

static int lua_bus_tostring(lua_State * L) {
	bus self = lua_tobus(L, 1);
	lua_pushfstring(L, "bus (%p)", self);
	return 1;
}

static int lua_bus_create(lua_State * L) {
	bus self = bus_create();
	if (!self) {
		return luaL_error(L, "failed to allocate bus");
	}
		
	bus * u = (bus *)lua_newuserdata(L, sizeof(bus));
	*u = self;
	luaL_getmetatable(L, "meta_bus");
	lua_setmetatable(L, -2);
	return 1;
}

bus lua_pushbus(lua_State * L, bus self) {
	/* this kind of bus won't be freed by gc: */
	((process)self)->freefunc = bus_nofree_msg;
		
	bus * u = (bus *)lua_newuserdata(L, sizeof(bus));
	*u = self;
	luaL_getmetatable(L, "meta_bus");
	lua_setmetatable(L, -2);
	return self;
}

int lua_audio_init(lua_State * L) {
	/* ensure it exists */
	audiomain = delta_main_get();
	
	/* define bus metatable */
	struct luaL_reg meta_bus_lib[] = {
		{ "__tostring", lua_bus_tostring },
		{ "__gc", lua_audio_gc },
		{ "close", lua_audio_gc },
		{ NULL, NULL }
	};
	luaL_newmetatable(L, "meta_bus");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, meta_bus_lib);
	lua_pushboolean(L, 1);	
	lua_setfield(L, -2, AUDIO_BUS_LITERAL);	
	lua_pop(L, 1);
	
	/* create a creator */
	lua_pushcfunction(L, lua_bus_create);
	lua_setglobal(L, "bus");
	
	/* make the main IO public: */
	for (int i=0; i<AUDIO_INPUTS; i++) {
		lua_pushfstring(L, "in%d", i+1);
		lua_pushbus(L, audiomain->inputs[i]);
		lua_settable(L, LUA_GLOBALSINDEX); 
	}
	for (int i=0; i<AUDIO_OUTPUTS; i++) {
		lua_pushfstring(L, "out%d", i+1);
		lua_pushbus(L, audiomain->outputs[i]);
		lua_settable(L, LUA_GLOBALSINDEX); 
	}
	
	return 0;
}