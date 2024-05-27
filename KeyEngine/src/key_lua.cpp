#include "key_lua.h"


bool luaCheck( lua_State *luaVm,
	const int ret )
{
	if ( ret != LUA_OK )
	{
		std::string errorMsg = lua_tostring( luaVm, -1 );
		std::cout << errorMsg << '\n';
		return false;
	}
	return true;
}

void executeLuaGetNumber( lua_State *luaVm,
	const std::string &strVar )
{
	int ret = luaL_dostring(luaVm, strVar.c_str());
	ASSERT( luaCheck( luaVm, ret), "Lua error!" );

	// if executed successfully the result "a" exists globally in the Lua vm
	lua_getglobal(luaVm, "a");	// pushes "a" to the stack and Lua replaces it, if it finds it, with the contents of "a"
	// lua communicates with C via the Lua stack
	// -1 is the Last entered position in the stack (Lua uses 1-indexing)
	if ( lua_isnumber(luaVm, -1) )
	{
		float a_c = (float)lua_tonumber(luaVm, -1);
		std::cout << "a=" << a_c << '\n';
	}
	std::cout << '\n';
}

void executeLuaGetString( lua_State *luaVm,
	const std::string &strVar )
{
	int ret = luaL_dostring(luaVm, strVar.c_str());
	ASSERT( luaCheck( luaVm, ret), "Lua error!" );

	lua_getglobal(luaVm, "PlayerName");
	if ( lua_isstring(luaVm, -1) )
	{
		std::string str_c = (std::string)lua_tostring(luaVm, -1);
		std::cout << "PlayerName=" << str_c << '\n';
	}
	std::cout << '\n';
}

void executeLuaFileGetNumber( lua_State *luaVm,
	const std::string &filename )
{
	int ret = luaL_dofile(luaVm, filename.c_str());
	ASSERT( luaCheck( luaVm, ret), "Lua error!" );

	lua_getglobal(luaVm, "file_result");
	if ( lua_isnumber(luaVm, -1) )
	{
		float file_result_c = (float)lua_tonumber(luaVm, -1);
		std::cout << "file_result=" << file_result_c << '\n';
	}
	std::cout << '\n';
}

void executeLuaFileGetTable( lua_State *luaVm,
	const std::string &filename,
	const std::string &tableName )
{
	struct Player
	{
		std::string title;
		std::string name;
		std::string family;
		int level;
	} player;

	int ret = luaL_dofile(luaVm, filename.c_str());
	ASSERT( luaCheck( luaVm, ret), "Lua error!" );

	lua_getglobal(luaVm, tableName.c_str());
	if ( lua_istable(luaVm, -1) )
	{
		lua_pushstring(luaVm, "Name");	// pushes a lua string variable at the top of the stack
		lua_gettable(luaVm, -2);	// since "name" was pushed to the top of the stack (@ index = -1) tableName is now @ index = -2
		// lua_getttable searches for (by means of key-value pair) the parameter with name located at the top of the lua stack pops it from the to of the stack and replaces it with its value
		player.name = lua_tostring(luaVm, -1);	// so now we get that value
		lua_pop(luaVm, 1);						// pop top of the stack string variable so now we can search for the new key of the table we want to query
		// notice for pop()ping the index reverses

		lua_pushstring(luaVm, "Title");
		lua_gettable(luaVm, -2);
		player.title = lua_tostring(luaVm, -1);
		lua_pop(luaVm, 1);

		lua_pushstring(luaVm, "Family");
		lua_gettable(luaVm, -2);
		player.family = lua_tostring(luaVm, -1);
		lua_pop(luaVm, 1);

		lua_pushstring(luaVm, "Level");
		lua_gettable(luaVm, -2);
		player.level = (int) lua_tonumber(luaVm, -1);
		lua_pop(luaVm, 1);

		std::cout << player.title << " " << player.name << " " << player.family << " " << player.level;
	}
	std::cout << '\n';
}

void executeLuaFunctionFromFile( lua_State *luaVm,
	const std::string &filename,
	const std::string &functionName )
{
	int ret = luaL_dofile(luaVm, filename.c_str());
	ASSERT( luaCheck( luaVm, ret), "Lua error!" );

	lua_getglobal( luaVm, functionName.c_str() );	// gets the global lua object and puts it in the lua stack
	if ( lua_isfunction(luaVm, -1) )
	{
		// push 2 args to the stack for the lua function to receive
		lua_pushnumber( luaVm, 10.19f );
		lua_pushnumber( luaVm, 33.01f );

		ret = lua_pcall(luaVm,
			2,	// how many args we're passing to the lua func
			1,	// how many args we're returning from the lua func
			0	// error checking
		);
		ASSERT( luaCheck( luaVm, ret ), "Lua function error!" );
		
		// the return value of the function is, as usual, at the top of the stack has
		const float function_result_c = (float)lua_tonumber(luaVm, -1);

		std::cout << "[C++:] " << functionName << "(a, b) returned " << function_result_c << '\n';
	}

	std::cout << '\n';
}

extern "C" int callableFromLua_mySum( lua_State *luaVm )
{
	float a = (float) lua_tonumber( luaVm, 1 );
	float b = (float) lua_tonumber( luaVm, 2 );
	std::cout << "[C++:] callableFromLua_mySum(" << a << ", " << b << ") called from Lua" << '\n';
	
	float c = a * b;
	lua_pushnumber( luaVm, c );

	return 1;	// returns # of arguments passed to Lua
}