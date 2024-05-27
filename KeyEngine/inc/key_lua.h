#pragma once

#include <string>
#include <iostream>
#include <assertions_console.h>

extern "C"
{
// Lua is a C library
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

#pragma comment( lib, "lua53.lib" )


bool luaCheck( lua_State *luaVm, const int ret );
void executeLuaGetNumber( lua_State *luaVm, const std::string &strVar );
void executeLuaGetString( lua_State *luaVm, const std::string &strVar );
void executeLuaFileGetNumber( lua_State *luaVm, const std::string &filename );
void executeLuaFileGetTable( lua_State *luaVm, const std::string &filename, const std::string &tableName );
void executeLuaFunctionFromFile( lua_State *luaVm, const std::string &filename, const std::string &functionName );

template<typename TCallable>
void executeLuaCFunctionFromFile( lua_State *luaVm,
	const std::string &filename,
	const std::string &luaFunctionName,
	const std::string &cFunctionName,
	TCallable f )
{
	lua_register(luaVm, cFunctionName.c_str(), f);

	int ret = luaL_dofile(luaVm, filename.c_str());
	ASSERT( luaCheck( luaVm, ret), "Lua error!" );

	lua_getglobal( luaVm, luaFunctionName.c_str() );	// gets the global lua object and puts it in the lua stack
	if ( lua_isfunction(luaVm, -1) )
	{
		lua_pushnumber( luaVm, 1.0f );
		lua_pushnumber( luaVm, 2.0f );

		ret = lua_pcall(luaVm, 2, 1, 0);
		ASSERT( luaCheck( luaVm, ret ), "Lua function error!" );
		
		// the return value of the function is, as usual, at the top of the stack has
		const float function_result_c = (float)lua_tonumber(luaVm, -1);

		std::cout << "[C++:] " << luaFunctionName << "(a, b) returned " <<  function_result_c << '\n';
	}

	std::cout << '\n';
}

/// C++ functions callable from Lua:
extern "C" int callableFromLua_mySum( lua_State *luaVm );