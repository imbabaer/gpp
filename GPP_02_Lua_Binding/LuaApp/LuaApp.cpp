#include "stdafx.h"

int main(int argc, char* argv[])
{
	const char* filename = argv[1];
	printf("%s\n",filename);
	// create a new Lua state
	lua_State* L = luaL_newstate();

	// open all Lua standard libraries
	luaL_openlibs(L);

	// execute a string as Lua code
	luaL_dostring(L, "print('hello from Lua!')\n");

	// close the Lua state
	lua_close(L);

	return 0;
}
