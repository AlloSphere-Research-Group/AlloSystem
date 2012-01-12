#ifndef INC_AL_LUA_HPP
#define INC_AL_LUA_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS).
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	Utilities for working with Lua

	File author(s):
	Graham Wakefield, 2012, grrrwaaa@gmail.com
*/

#include "lua.hpp"

namespace al {

///! A wrapper around the lua_State:
class Lua {
public:

	Lua() : L(0) {
		L = lua_open();
		luaL_openlibs(L);
	}

	///! destructor calls
	~Lua() {
		if (L) {
			lua_close(L);
			L = 0;
		}
	}

	Lua& setglobal(const std::string& name, std::string& value) {
        lua_pushstring(L, value.c_str());
        lua_setglobal(L, name.c_str());
        return *this;
	}

	int lerror(lua_State * L, int err) {
		if (err) {
			printf("error: %s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
		return err;
	}

	int pcall(lua_State * L, int nargs) {
		int debug_idx = -nargs-3;
		// put debug.traceback just below the function & args
		{
			lua_getglobal(L, "debug");
			lua_pushliteral(L, "traceback");
			lua_gettable(L, -2);
			lua_insert(L, debug_idx);
			lua_pop(L, 1); // pop debug table
		}
		int top = lua_gettop(L);
		int res = lerror(L, lua_pcall(L, nargs, LUA_MULTRET, -nargs-2));
		int nres = lua_gettop(L) - top;
	//	int nres = lua_gettop(L) - top + nargs + 1;
		lua_remove(L, -(nres+nargs+2)); // remove debug function from stack
		return res;
	}

	int dostring(const std::string& code, int nargs=0) {
		return lerror(L, luaL_loadstring(L, code.c_str())) || pcall(L, nargs);
	}

	int dofile(const std::string& path, int nargs=0) {
		return lerror(L, luaL_loadfile(L, path.c_str())) || pcall(L, nargs);
	}

	///! allow the Lua object to be used in place of a lua_State *
	operator lua_State *() { return L; }
	operator const lua_State *() { return L; }

protected:
	lua_State * L;
};

} // al::

#endif
