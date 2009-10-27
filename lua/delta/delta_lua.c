#include "delta_lua.h"
#include "math.h"

#define ddebug(...) 
//#define ddebug(...) printf(__VA_ARGS__)

static const char * DELTA_LUA_CORO_CACHE = "coro_cache";
static const char * DELTA_LUA_CORO_META = "coro_meta";

/*
	Routine for coroutines
 */
static int resume(al_sec t, char * data) {
	lua_State * C = *(lua_State **)data;
	
	ddebug("resume %f\n", t);
	
	// resume the coroutine
	int result;
	switch(lua_status(C)) {
		case 0:
			result = lua_resume(C, lua_gettop(C) - 1); 
			break;
		case LUA_YIELD:
			result = lua_resume(C, lua_gettop(C)); 
			break;
		default:
			result = lua_status(C);
	}
	
	if (result != LUA_YIELD) {
		// unref the task to allow __gc
		lua_getfield(C, LUA_REGISTRYINDEX, DELTA_LUA_CORO_CACHE);
		lua_pushthread(C);
		lua_pushnil(C);
		lua_rawset(C, -3);
		lua_pop(C, 1);
	
		// handle errors:
		switch (result) {
			case 0: // coroutine completed
				break;
			case LUA_ERRRUN:
				fprintf(stderr, "runtime error %s\n", lua_tostring(C, -1));
				lua_pop(C, 1);
				break;
			case LUA_ERRSYNTAX:
				fprintf(stderr, "syntax error %s\n", lua_tostring(C, -1));
				lua_pop(C, 1);
				break;
			case LUA_ERRMEM:
				fprintf(stderr, "memory error %s\n", lua_tostring(C, -1));
				break;
			default:
				fprintf(stderr, "unknown error %s\n", lua_tostring(C, -1));
		}	
	}
	return 0;
}

static int coro_tostring(lua_State * C) {
	lua_pushfstring(C, "scheduled coroutine (%p)", lua_tothread(C, -1));
	return 1;
}

// __gc of task should remove it from the schedule.
static int coro_gc(lua_State * C) {
	printf("gc of scheduled coroutine %p\n", C);
	return 1;
}

static int coro_abort(lua_State * L) {
	if (!lua_isthread(L, -1)) {
		lua_pushthread(L);
	}
	// leave 2 copies of the thread on the stack:
	lua_pushvalue(L, -1);
	
	// use first copy to retrieve associated clock:
	lua_rawget(L, lua_upvalueindex(1));	// DELTA_LUA_CORO_CACHE
	pq * u = (pq *)lua_touserdata(L, -1);
	if (u) {
		al_pq_cancel_ptr(*u, lua_tothread(L, -2));
	}
	lua_pop(L, 1);
	
	// use 2nd copy to remove from registry (allow __gc)
	lua_pushnil(L);
	lua_rawset(L, lua_upvalueindex(1));	// DELTA_LUA_CORO_CACHE
	
	return 0;
}

static int scheduler_tostring(lua_State * L) {
	lua_pushfstring(L, "delta.scheduler (%p)", *(pq **)lua_touserdata(L, 1));
	return 1;
}

static int scheduler_gc(lua_State * L) {
	pq scheduler = *(pq *)lua_touserdata(L, 1);
	al_pq_free(&scheduler, 0);
	// ensure we can't double-free
	lua_pushnil(L);
	lua_setmetatable(L, -2);
	return 0;
}

static int scheduler_close(lua_State * L) {
	lua_pushvalue(L, lua_upvalueindex(1));
	return scheduler_gc(L);
}

static int scheduler_go(lua_State * L) {
	lua_State ** buf;
	pq scheduler = *(pq *)lua_touserdata(L, lua_upvalueindex(1));
	
	al_sec t = 0; 
	if (lua_isnumber(L, 1)) {
		t = (al_sec)lua_tonumber(L, 1);
		lua_remove(L, 1);
	}	
	
	// create a new coro:
	lua_State * C = lua_newthread(L);
	luaL_getmetatable(L, DELTA_LUA_CORO_META);
	lua_setmetatable(L, -2);
	
	// insert a userdata proxy to get __gc callbacks on the coro:
	lua_State ** proxy = (lua_State **)lua_newuserdata(C, sizeof(lua_State *));
	*proxy = C;
	luaL_getmetatable(C, DELTA_LUA_CORO_META);
	lua_setmetatable(C, -2);
	// store in a hidden place:
	lua_pushthread(C);
	lua_settable(C, LUA_REGISTRYINDEX);
	
	lua_xmove(L, C, lua_gettop(L));
	
	// return 'me' to L, the return value of 'go':
	lua_xmove(C, L, 1);	
	
	// register with scheduler (prevents __gc, and provides a reference back to scheduler when the coro yields)
	lua_pushvalue(L, -1);
	lua_pushvalue(L, lua_upvalueindex(1));	// this Scheduler
	lua_rawset(L, lua_upvalueindex(2));	// DELTA_LUA_CORO_CACHE
	
	// insert C into the clock schedule at delay t
	t += scheduler->now;
	buf = (lua_State **)al_pq_msg(scheduler);
	if (buf) {
		// copy C into buf:
		*buf = C;
		al_pq_sched(scheduler, t, resume);
		ddebug("go scheduled %f\n", t);
	} else {
		return luaL_error(L, "go: failed to allocate message");
	}
	return 1;
}

static int scheduler_wait(lua_State * C) {
	// get the wait period (seconds) & convert to ns:
	lua_State ** buf;
	al_sec t = (al_sec)(fabs(luaL_optnumber(C, 1, 1.0)));
	pq scheduler = *(pq *)lua_touserdata(C, lua_upvalueindex(1));
	
	// register with this scheduler:
	lua_pushthread(C);
	lua_pushvalue(C, lua_upvalueindex(1));	// this Scheduler
	lua_rawset(C, lua_upvalueindex(2));	// DELTA_LUA_CORO_CACHE
	
	t += scheduler->now;
	buf = (lua_State **)al_pq_msg(scheduler);
	if (buf) {
		// copy C into buf:
		*buf = C;
		al_pq_sched(scheduler, t, resume);
		ddebug("wait scheduled %f\n", t);
	} else {
		return luaL_error(C, "wait: failed to allocate message");
	}
	
	// yield the coro:
	return lua_yield(C, 0);
}

static int scheduler_now(lua_State * L) {
	pq scheduler = *(pq *)lua_touserdata(L, lua_upvalueindex(1));
	lua_pushnumber(L, scheduler->now);
	return 1;
}

static int scheduler_update(lua_State * L) {
	al_sec until = (al_sec)(fabs(luaL_optnumber(L, 1, 1.)));
	int defer = lua_toboolean(L, 2);
	pq scheduler = *(pq *)lua_touserdata(L, lua_upvalueindex(1));
	al_pq_update(scheduler, until, defer);
	lua_pushnumber(L, scheduler->now);
	return 1;
}

static int scheduler_advance(lua_State * L) {
	al_sec period = (al_sec)(fabs(luaL_optnumber(L, 1, 1.)));
	int defer = lua_toboolean(L, 2);
	pq scheduler = *(pq *)lua_touserdata(L, lua_upvalueindex(1));
	al_pq_advance(scheduler, period, defer);
	lua_pushnumber(L, scheduler->now);
	return 1;
}

static int scheduler(lua_State * L) {	
	// TODO: grab size & birth args from stack?
	pq scheduler = al_pq_create(1000, 0);
	pq * u = (pq *)lua_newuserdata(L, sizeof(pq));
	*u = scheduler;
	
	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	
	lua_pushcfunction(L, scheduler_tostring);
	lua_setfield(L, -2, "__tostring");
	
	lua_pushcfunction(L, scheduler_gc);
	lua_setfield(L, -2, "__gc");
	
	lua_pushvalue(L, -2);
	lua_pushcclosure(L, scheduler_close, 1);
	lua_setfield(L, -2, "close");
	
	lua_pushvalue(L, -2);
	lua_pushcclosure(L, scheduler_now, 1);
	lua_setfield(L, -2, "now");
	
	lua_pushvalue(L, -2);
	lua_getfield(L, LUA_REGISTRYINDEX, DELTA_LUA_CORO_CACHE);
	lua_pushcclosure(L, scheduler_go, 2);
	lua_setfield(L, -2, "go");
	
	lua_pushvalue(L, -2);
	lua_pushcclosure(L, scheduler_update, 1);
	lua_setfield(L, -2, "update");
	
	lua_pushvalue(L, -2);
	lua_pushcclosure(L, scheduler_advance, 1);
	lua_setfield(L, -2, "advance");
	
	lua_pushvalue(L, -2);
	lua_getfield(L, LUA_REGISTRYINDEX, DELTA_LUA_CORO_CACHE);
	lua_pushcclosure(L, scheduler_wait, 2);
	lua_setfield(L, -2, "wait");
	
//	lua_pushvalue(L, -2);
//	lua_pushcclosure(L, scheduler_clear, 1);
//	lua_setfield(L, -2, "clear");
	
	lua_setmetatable(L, -2);
	return 1;
}

//int cache(lua_State * L) {
//	lua_getfield(L, LUA_REGISTRYINDEX, DELTA_LUA_CORO_CACHE);
//	return 1;
//}	

int luaopen_delta(lua_State * L) {

	delta_main_init();

	const char * libname = lua_tostring(L, -1);
	
	// create weak-valued Scheduler lookup table:
	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setfield(L, LUA_REGISTRYINDEX, DELTA_LUA_CORO_CACHE);
	
	/*
	 create metatable for coroutines created by the Scheduler:
	 */
	luaL_newmetatable(L, DELTA_LUA_CORO_META);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, coro_tostring);
	lua_setfield(L, -2, "__tostring");
	/* seems like __gc is not called for coroutines; need a proxy instead
	lua_pushcfunction(L, coro_gc);
	lua_setfield(L, -2, "__gc"); 
	 */
	// the coro functions use the Scheduler table as upvalueindex(1) for rapid access:
	lua_getfield(L, LUA_REGISTRYINDEX, DELTA_LUA_CORO_CACHE);
	lua_pushcclosure(L, coro_abort, 1);
	lua_setfield(L, -2, "abort");
	// clear Coro metatable:
	lua_pop(L, 1);	
	
	/*
	 create the time module table
	 */
	struct luaL_reg lib[] = {
		//{"cputime", cputime },
		{"scheduler", scheduler },
		//{"cache", cache },
		{NULL, NULL},
	};
	luaL_register(L, libname, lib);
	
	/*
	 extend it with closures on the main scheduler userdata
	 */
	pq * u = (pq *)lua_newuserdata(L, sizeof(pq));
	// TODO: store this as singleton instance, or use existing singleton?
	*u = al_pq_main(); //al_pq_create(10, 0);
	
	lua_pushvalue(L, -2); // the module table also becomes the metatable of the main scheduler
	lua_pushvalue(L, -2);
	lua_pushcclosure(L, scheduler_now, 1);
	lua_setfield(L, -2, "now");
	
	lua_pushvalue(L, -2);
	lua_getfield(L, LUA_REGISTRYINDEX, DELTA_LUA_CORO_CACHE);
	lua_pushcclosure(L, scheduler_go, 2);
	lua_setfield(L, -2, "go");
	
	lua_pushvalue(L, -2);
	lua_getfield(L, LUA_REGISTRYINDEX, DELTA_LUA_CORO_CACHE);
	lua_pushcclosure(L, scheduler_wait, 2);
	lua_setfield(L, -2, "wait");
	
//	lua_pushvalue(L, -2);
//	lua_pushcclosure(L, scheduler_clear, 1);
//	lua_setfield(L, -2, "clear");	
	lua_setmetatable(L, -2);
	
	return 1;
}