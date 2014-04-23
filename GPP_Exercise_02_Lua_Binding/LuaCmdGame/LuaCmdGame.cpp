#include "stdafx.h"
#include <fstream>

#define NUM_KEYS (VK_OEM_CLEAR + 1)
std::vector<bool> KeysDown;
std::vector<bool> KeysPressed;
DWORD sleepTime = 100;
HANDLE hConsole;
COORD cursorHomePosition = { 0, 0 };
enum { EXIT_GAME = 0, CONTINUE_GAME };

int lua_CFunction_isKeyDown(lua_State* L)
{
	// TODO implement lua_CFunction_isKeyDown
	
	int n = lua_gettop(L);
	if(n != 1) printf("error: expected one argument\n");
	// get the first argument
	if(!lua_isnumber(L, -1)) printf("error: expected number\n");
	float x = (float)lua_tonumber(L, -1);

	// push the return value on the stack
	lua_pushboolean(L, KeysDown.at(x));
	// the function returns 1 value
	
	return 1;
}

int lua_CFunction_wasKeyPressed(lua_State* L)
{
	// TODO implement lua_CFunction_wasKeyPressed

	int n = lua_gettop(L);
	if(n != 1) printf("error: expected one argument\n");
	// get the first argument
	if(!lua_isnumber(L, -1)) printf("error: expected number\n");
	float x = (float)lua_tonumber(L, -1);

	// push the return value on the stack
	lua_pushboolean(L, KeysPressed.at(x));
	// the function returns 1 value
	
	return 1;
}

int lua_CFunction_setSleepTime(lua_State* L)
{
	// TODO implement lua_CFunction_setSleepTime
	int n = lua_gettop(L);
	if(n != 1) printf("error: expected one arguments\n");
	// get the first argument
	if(!lua_isnumber(L, -1)) printf("error: expected number\n");
	::sleepTime = (unsigned long)lua_tonumber(L, -1);

	//::sleepTime;
	return 0;
}

void printLuaError(lua_State* L)
{
// the top of the stack *should* now contain the error message
const char* theError = lua_tostring(L, lua_gettop(L));
lua_pop(L, 1);
printf(theError); printf("\n");
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

	// TODO create a new Lua state
	lua_State* L = luaL_newstate();

	// TODO open all Lua standard libraries
	luaL_openlibs(L);

	// TODO load and execute the Lua file
	const char* filename = argv[1];
	//int e1 = luaL_loadfile(L, filename);
	//int e2 = lua_pcall(L,0,-1,0);
	std::ifstream file;
	file.open(filename, std::ios::binary);
	file.seekg(0,std::ios::end);
	size_t fileSize = (size_t)file.tellg().seekpos();

	luaL_dofile(L,filename);

	// initialize the keyboard states
	KeysDown.resize(NUM_KEYS);
	KeysPressed.resize(NUM_KEYS);
	for (int key = 1; key < NUM_KEYS; ++key)
	{
		KeysDown[key] = false;
		KeysPressed[key] = false;
	}

	// TODO register all lua_CFunctions
	lua_pushcfunction(L, lua_CFunction_isKeyDown);
	lua_setglobal(L, "isKeyDown");
	lua_pushcfunction(L, lua_CFunction_wasKeyPressed);
	lua_setglobal(L, "wasKeyPressed");
	lua_pushcfunction(L, lua_CFunction_setSleepTime);
	lua_setglobal(L, "setSleepTime");

	// TODO register global exit codes
	lua_pushnumber(L, EXIT_GAME);
	lua_setglobal(L, "EXIT_GAME");
	lua_pushnumber(L, CONTINUE_GAME);
	lua_setglobal(L, "CONTINUE_GAME");

	// TODO call the Lua function initialize()
	lua_getglobal(L, "initialize");
	int err = lua_pcall(L,0,0,0);
	if(err != LUA_OK) printLuaError(L);

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

		// TODO call the Lua function update()
		lua_getglobal(L, "update");
		if (!lua_isfunction(L,-1)) printf("no function");
		lua_call(L,0,1);

		// TODO get the return value from update()
		int returnValue = CONTINUE_GAME;
		if (!lua_isnumber(L,-1))
		{
			returnValue = lua_tointeger(L,-1);
		}

		if (returnValue == EXIT_GAME)
			break;

		// reset cursor to 0,0
		SetConsoleCursorPosition(hConsole, cursorHomePosition);

		// TODO call the Lua function draw()
		lua_getglobal(L, "draw");
		lua_call(L,0,0);


		// sleep for a few milliseconds
		Sleep(sleepTime);
	}

	// TODO close the Lua state
	lua_close(L);

	return 0;
}
