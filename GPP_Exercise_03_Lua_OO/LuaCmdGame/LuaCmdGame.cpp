#include "stdafx.h"
#include "Player.h"

#define NUM_KEYS (VK_OEM_CLEAR + 1)
std::vector<bool> KeysDown;
std::vector<bool> KeysPressed;
DWORD sleepTime = 100;
HANDLE hConsole;
COORD cursorHomePosition = { 0, 0 };
enum { EXIT_GAME = 0, CONTINUE_GAME };

int lua_CFunction_isKeyDown(lua_State* L)
{
	int n = lua_gettop(L);
	if (n != 1)
		luaL_error(L, "function \"isKeyDown\" expected 1 parameter but got %i", n);
	if (!lua_isnumber(L, 1))
		luaL_error(L, "function \"isKeyDown\" expected type number as first argument");
	SHORT keyCode = (SHORT)lua_tonumber(L, 1);
	if (keyCode<VK_LBUTTON || keyCode>VK_OEM_CLEAR)
		luaL_error(L, "function \"isKeyDown\" parameter keyCode must be between %i and %i", VK_LBUTTON, VK_OEM_CLEAR);
	lua_pushboolean(L, KeysDown[keyCode] ? TRUE : FALSE);
	return 1;
}

int lua_CFunction_wasKeyPressed(lua_State* L)
{
	int n = lua_gettop(L);
	if (n != 1)
		luaL_error(L, "function \"wasKeyPressed\" expected 1 parameter but got %i", n);
	if (!lua_isnumber(L, 1))
		luaL_error(L, "function \"wasKeyPressed\" expected type number as first argument");
	SHORT keyCode = (SHORT)lua_tonumber(L, 1);
	if (keyCode<VK_LBUTTON || keyCode>VK_OEM_CLEAR)
		luaL_error(L, "function \"wasKeyPressed\" parameter keyCode must be between %i and %i", VK_LBUTTON, VK_OEM_CLEAR);
	lua_pushboolean(L, KeysPressed[keyCode] ? TRUE : FALSE);
	return 1;
}

int lua_CFunction_setSleepTime(lua_State* L)
{
	int n = lua_gettop(L);
	if (n != 1)
		luaL_error(L, "function \"setSleepTime\" expected 1 parameter but got %i", n);
	if (!lua_isnumber(L, 1))
		luaL_error(L, "function \"setSleepTime\" expected type number as first argument");
	DWORD sleepTime = (int)lua_tonumber(L, 1);
	if (sleepTime < 0)
		luaL_error(L, "function \"setSleepTime\" parameter sleepTime must be larger than 0");
	::sleepTime = sleepTime;
	return 0;
}

void Exercise_03_Lua_OO_UnitTest(lua_State* L)
{
	int err = -1;

	Vec2::lua_bind(L);
	lua_getglobal(L, "test_Vec2");
	lua_call(L, 0, 1);
	err = (int)lua_tointeger(L, -1);
	if (err != LUA_OK)
	{
		printf("\ntest_Vec2 failed! Fix all the errors before you can play the game.\n");
		std::cin.get();
		exit(-1);
	}
	
	Player::lua_bind(L);
	lua_getglobal(L, "test_Player");
	lua_call(L, 0, 1);
	err = (int)lua_tointeger(L, -1);
	if (err != LUA_OK)
	{
		printf("\ntest_Player failed! Fix all the errors before you can play the game.\n");
		std::cin.get();
		exit(-1);
	}

	printf("press any key to continue");
	std::cin.get();
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("-------------------- LuaCmdGame.exe --------------------\n");
		printf("Please specify a .lua file to be processed, for example:\n");
		printf("  LuaCmdGame.exe EmptyCmdGame.lua\n");
		printf("--------------------------------------------------------\n");
		system("pause");
		return 0;
	}

	// create a new Lua state
	lua_State* L = luaL_newstate();

	// open all Lua standard libraries
	luaL_openlibs(L);

	// load and execute the Lua file
	const char* filename = argv[1];

	// code from: http://www.gamedev.net/topic/501060-lua-dofile-error-output-as-c-string/?view=findpost&p=4269077
	{
		int err = luaL_loadfile(L, filename);
		if(err != LUA_OK)
		{
			// the top of the stack should be the error string
			if (lua_isstring(L, lua_gettop(L)))
			{
				// get the top of the stack as the error and pop it off
				const char* theError = lua_tostring(L, lua_gettop(L));
				lua_pop(L, 1);

				luaL_error(L, theError);
			}
			else
			{
				luaL_error(L, "Unknown error loading Lua file \"%s\"", filename);
			}
		}
		else
		{
			// if not an error, then the top of the stack will be the function to call to run the file
			err = lua_pcall(L, 0, LUA_MULTRET, 0);
			if (err != LUA_OK)
			{
				// the top of the stack should be the error string
				if (lua_isstring(L, lua_gettop(L)))
				{
					// get the top of the stack as the error and pop it off
					const char* theError = lua_tostring(L, lua_gettop(L));
					lua_pop(L, 1);

					luaL_error(L, theError);
				}
				else
				{
					luaL_error(L, "Unknown error executing Lua file \"%s\"", filename);
				}
			}
		}
	}

	// start the unit test for Exercise 03 - Lua OO
	Exercise_03_Lua_OO_UnitTest(L);

	// initialize the keyboard states
	KeysDown.resize(NUM_KEYS);
	KeysPressed.resize(NUM_KEYS);
	for (int key = 1; key < NUM_KEYS; ++key)
	{
		KeysDown[key] = false;
		KeysPressed[key] = false;
	}

	// register all lua_CFunctions
	lua_pushcfunction(L, lua_CFunction_isKeyDown);
	lua_setglobal(L, "isKeyDown");
	lua_pushcfunction(L, lua_CFunction_wasKeyPressed);
	lua_setglobal(L, "wasKeyPressed");
	lua_pushcfunction(L, lua_CFunction_setSleepTime);
	lua_setglobal(L, "setSleepTime");

	// register global exit codes
	lua_pushnumber(L, EXIT_GAME);
	lua_setglobal(L, "EXIT_GAME");
	lua_pushnumber(L, CONTINUE_GAME);
	lua_setglobal(L, "CONTINUE_GAME");

	// call the Lua function initialize()
	lua_getglobal(L, "initialize");
	if (!lua_isfunction(L, -1))
		luaL_error(L, "function \"initialize\" was not found");
	lua_call(L, 0, 0);

	// get the console handle
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	system("cls");

	// game loop
	while (!KeysDown[VK_ESCAPE])
	{
		// read keyboard input
		for (int key = 1; key < NUM_KEYS; ++key)
		{
			KeysPressed[key] = false;
			bool keyDown = GetAsyncKeyState(key) != FALSE;
			if (KeysDown[key] != keyDown)
				KeysPressed[key] = true;
			KeysDown[key] = keyDown;
		}

		// call the Lua function update()
		lua_getglobal(L, "update");
		if (!lua_isfunction(L, -1))
			luaL_error(L, "function \"update\" was not found");
		lua_call(L, 0, 1);
		// get the return value from update()
		if (!lua_isnumber(L, -1))
			luaL_error(L, "function \"update\" expected one return value of type number");
		int returnValue = (int)lua_tonumber(L, -1);
		if (returnValue == EXIT_GAME)
			break;

		// reset cursor to 0,0
		SetConsoleCursorPosition(hConsole, cursorHomePosition);

		// call the Lua function draw()
		lua_getglobal(L, "draw");
		if (!lua_isfunction(L, -1))
			luaL_error(L, "function \"draw\" was not found");
		lua_call(L, 0, 0);

		// sleep for a few milliseconds
		Sleep(sleepTime);
	}

	// destroy the player
	Player::destroy();

	// close the Lua state
	lua_close(L);

	return 0;
}
