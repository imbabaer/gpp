#include "stdafx.h"
#include "Player.h"

Player* Player::s_pInstance = nullptr;

Player* Player::getInstance()
{
	if (s_pInstance == nullptr)
		s_pInstance = new Player;
	return s_pInstance;
}

void Player::destroy()
{
	delete(s_pInstance);
	s_pInstance = nullptr;
}

const char* Player::getName() const
{
	return m_name.c_str();
}

void Player::setName(const std::string& name)
{
	m_name = name;
}

Vec2 Player::getPosition() const
{
	return m_position;
}

void Player::setPosition(const Vec2& position)
{
	m_position = position;
}

void Player::lua_bind(lua_State* L)
{
	// TODO: create a new meta table "PlayerMeta"
	luaL_newmetatable(L, "PlayerMeta");
	lua_pushvalue(L, -1); 

	// TODO Done: set the field "__index" properly
	lua_setfield(L, -2, "__index"); 

	// TODO: add the functions "getName", "setName", "getPosition", and "setPosition" to the meta table
	lua_pushcfunction(L, lua_CFunction_getName); 
	lua_setfield(L, -2, "getName");
	lua_pushcfunction(L, lua_CFunction_setName); 
	lua_setfield(L, -2, "setName");
	lua_pushcfunction(L, lua_CFunction_getPosition); 
	lua_setfield(L, -2, "getPosition");
	lua_pushcfunction(L, lua_CFunction_setPosition); 
	lua_setfield(L, -2, "setPosition");



	// TODO: register a global player instance called "Player"
	// the player pointer can be obtained via "getInstance()"
	lua_newtable(L);
	/*lua_pushlightuserdata(L,Player::getInstance());
	lua_setfield(L,-2,"__ptr");*/
	lua_pushstring(L, "__ptr");
	lua_pushlightuserdata(L, Player::getInstance());
	lua_settable(L, -3);
	luaL_setmetatable(L,"PlayerMeta");

	lua_setglobal(L, "Player");

}

int Player::lua_CFunction_getName(lua_State* L)
{
	// TODO: create "Player* pSelf" from the stack
	lua_getfield(L,1,"__ptr");
	Player* pSelf = (Player*)lua_topointer(L,-1);
	lua_pop(L,1);
	// TODO: push "pSelf->getName()" on the stack
	if(pSelf!=nullptr)
	{
		lua_pushstring(L,pSelf->getName());
	}

	return 1;
}

int Player::lua_CFunction_setName(lua_State* L)
{
	// TODO: create "Player* pSelf" from the stack
	lua_getfield(L,1,"__ptr");
	Player* pSelf = (Player*)lua_topointer(L,-1);
	lua_pop(L,1);
	// TODO: get "const char* name" from the stack
	//lua_getfield(L,1,"getName");
	char const * name = (char const *)lua_tostring(L,-1);

	// TODO: call "pSelf->setName(name)"
	pSelf->setName(name);

	return 0;
}

int Player::lua_CFunction_getPosition(lua_State* L)
{
	int n;
	n = lua_gettop(L);

	// TODO: create "Player* pSelf" from the stack
	lua_getfield(L,1,"__ptr");
	Player* pSelf = (Player*)lua_topointer(L,-1);
	lua_pop(L,1);
	// TODO: push "pSelf->getPosition()" on the stack
	if(pSelf!=nullptr)
	{
		//lua_pushtable(L,&(pSelf->getPosition()));

		float x = pSelf->getPosition().m_x;
		float y = pSelf->getPosition().m_y;
		lua_pushnumber(L, x);
		lua_pushnumber(L,y);
		pSelf->getPosition().lua_CFunction_Vec2(L);
		n = lua_gettop(L);

		//Vec2 v = Vec2(x, y); // create an instance of Vec2 on the C/C++ stack
		//lua_newtable(L); // push a new table on the stack
		//lua_pushnumber(L, x); // push the value of x on the stack
		//lua_setfield(L, -2, "x"); // store the value of x in the field "x"
		//lua_pushnumber(L, y); // push the value of y on the stack
		//lua_setfield(L, -2, "y"); // store the value of y in the field "y"
		//luaL_setmetatable(L, "Vec2Meta"); // set the proper metatable
	}
	return 1;
}

int Player::lua_CFunction_setPosition(lua_State* L)
{
	int n = lua_gettop(L);
	// TODO: create "Player* pSelf" from the stack
	lua_getfield(L,1,"__ptr");
	Player* pSelf = (Player*)lua_topointer(L,-1);
	//lua_pop(L,1);
	// TODO: create and initialize "Vec2 position" from the stack
	float x =0;
	float y =0;
	n = lua_gettop(L);

	if( lua_istable(L, -1) )
	{
		//lua_gettable(L,1);
		//Vec2 position = ()
		lua_getfield(L, 1, "x");
		x = (float)lua_tonumber(L, -1); // get the constructor parameters from the stack
		lua_pop(L, 1); // pop the value of "x" from the stack
		lua_getfield(L, 1, "y");
		y = (float)lua_tonumber(L, -1); // TODO should also work for tables {x, y}
		lua_pop(L, 1); // pop the value of "y" from the stack
	}
	n = lua_gettop(L);

	Vec2 position(x, y);
	// TODO: call "pSelf->setPosition(position)"
	pSelf->setPosition(position);
	return 0;
}
