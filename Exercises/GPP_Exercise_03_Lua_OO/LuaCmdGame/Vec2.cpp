#include "stdafx.h"
#include "Vec2.h"

Vec2::Vec2() : 
	m_x(0.f),
	m_y(0.f)
{
}

Vec2::Vec2(float x, float y) : 
	m_x(x),
	m_y(y)
{

}

float Vec2::length() const
{
	return (float)sqrt(m_x*m_x + m_y*m_y);
}

Vec2 Vec2::operator+(const Vec2& other) const
{
	Vec2 result;
	result.m_x = m_x + other.m_x;
	result.m_y = m_y + other.m_y;
	return result;
}

Vec2 Vec2::operator-(const Vec2& other) const
{
	Vec2 result;
	result.m_x = m_x - other.m_x;
	result.m_y = m_y - other.m_y;
	return result;
}

void Vec2::lua_bind(lua_State* L)
{
	// TODO DOne: create a new meta table "Vec2Meta"
	luaL_newmetatable(L, "Vec2Meta");
	lua_pushvalue(L, -1); 

	// TODO Done: set the field "__index" properly
	lua_setfield(L, -2, "__index"); 

	// TODO done: add the functions "length", "__add", and "__sub" to the meta table
	lua_pushcfunction(L, lua_CFunction_length); 
	lua_setfield(L, -2, "length");
	lua_pushcfunction(L, lua_CFunction_add); 
	lua_setfield(L, -2, "__add");
	lua_pushcfunction(L, lua_CFunction_sub); 
	lua_setfield(L, -2, "__sub");

	// TODO done: register a global function "Vec2" which serves as constructor
	lua_pushcfunction(L, lua_CFunction_Vec2); 
	lua_setglobal(L, "Vec2"); 
}

int Vec2::lua_CFunction_Vec2(lua_State* L)
{
	int n = lua_gettop(L);
	float x =0;
	float y =0;
	if( lua_istable(L, -1) )
	{
		lua_getfield(L, 1, "x");
		x = (float)lua_tonumber(L, -1); // get the constructor parameters from the stack
		lua_pop(L, 1); // pop the value of "x" from the stack
		lua_getfield(L, 1, "y");
		y = (float)lua_tonumber(L, -1); // TODO should also work for tables {x, y}
		lua_pop(L, 1); // pop the value of "x" from the stack
	}
	else 
	{
		x = (float)lua_tonumber(L, 1); // get the constructor parameters from the stack
		y = (float)lua_tonumber(L, 2); // TODO should also work for tables {x, y}
	}
	Vec2 v = Vec2(x, y); // create an instance of Vec2 on the C/C++ stack
	lua_newtable(L); // push a new table on the stack
	lua_pushnumber(L, x); // push the value of x on the stack
	lua_setfield(L, -2, "x"); // store the value of x in the field "x"
	lua_pushnumber(L, y); // push the value of y on the stack
	lua_setfield(L, -2, "y"); // store the value of y in the field "y"
	luaL_setmetatable(L, "Vec2Meta"); // set the proper metatable
	return 1;

	// TODO: create "Vec2 instance"
	// TODO: initialite "instance"
	//	either from a table {x, y}
	//	or from two numbers x, y
	// TODO: push "instance" on the stack

}

int Vec2::lua_CFunction_length(lua_State* L)
{
	// TODO: create and initialize "Vec2 self" from the stack
	// TODO: push "self.length()" on the stack
	//return 0;
	lua_getfield(L, 1, "x"); // read the field "x"
	float x = (float)lua_tonumber(L, -1); // get the value of "x" from the stack
	lua_pop(L, 1); // pop the value of "x" from the stack
	lua_getfield(L, 1, "y"); // read the field "y"
	float y = (float)lua_tonumber(L, -1); // get the value of "y" from the stack
	lua_pop(L, 1); // pop the value of "y" from the stack
	Vec2 v(x, y); // create an instance of Vec2 on the C/C++ stack
	lua_pushnumber(L, v.length()); // call the member function and push the result on the stack
	return 1;
}

int Vec2::lua_CFunction_add(lua_State* L)
{
	// TODO: create and initialize "Vec2 self" from the stack
	// TODO: create and initialize "Vec2 other" from the stack
	// TODO: calculate "Vec2 result = self + other"
	// TODO: push "result" on the stack
	//return 0;
	int n = lua_gettop(L);

	lua_getfield(L, 1, "x"); // read the field "x"
	n = lua_gettop(L);
	float x = (float)lua_tonumber(L, -1); // get the value of "x" from the stack
	n = lua_gettop(L);
	lua_pop(L, 1); // pop the value of "x" from the stack
	lua_getfield(L, 1, "y"); // read the field "y"
	float y = (float)lua_tonumber(L, -1); // get the value of "y" from the stack
	lua_pop(L, 1); // pop the value of "y" from the stack
	Vec2 v1(x, y); // create an instance of Vec2 on the C/C++ stack
	n = lua_gettop(L);


	lua_getfield(L, 2, "x"); // read the field "x"
	x = (float)lua_tonumber(L, -1); // get the value of "x" from the stack
	lua_pop(L, 1); // pop the value of "x" from the stack
	lua_getfield(L, 2, "y"); // read the field "y"
	y = (float)lua_tonumber(L, -1); // get the value of "y" from the stack
	lua_pop(L, 1); // pop the value of "y" from the stack
	Vec2 v2(x, y); // create an instance of Vec2 on the C/C++ stack
	n = lua_gettop(L);
	lua_pop(L, 1); 
	lua_pop(L, 1); 

	Vec2 result = v1 + v2;
	n = lua_gettop(L);

	lua_pushnumber(L, result.m_x);
	lua_pushnumber(L, result.m_y);
	n = lua_gettop(L);

	lua_CFunction_Vec2(L);
	return 1;
}

int Vec2::lua_CFunction_sub(lua_State* L)
{
	// TODO: create and initialize "Vec2 self" from the stack
	// TODO: create and initialize "Vec2 other" from the stack
	// TODO: calculate "Vec2 result = self - other"
	// TODO: push "result" on the stack
	// return 0;

		lua_getfield(L, 1, "x"); // read the field "x"
	float x = (float)lua_tonumber(L, -1); // get the value of "x" from the stack
	lua_pop(L, 1); // pop the value of "x" from the stack
	lua_getfield(L, 1, "y"); // read the field "y"
	float y = (float)lua_tonumber(L, -1); // get the value of "y" from the stack
	lua_pop(L, 1); // pop the value of "y" from the stack
	Vec2 v1(x, y); // create an instance of Vec2 on the C/C++ stack

	lua_getfield(L, 2, "x"); // read the field "x"
	x = (float)lua_tonumber(L, -1); // get the value of "x" from the stack
	lua_pop(L, 1); // pop the value of "x" from the stack
	lua_getfield(L, 2, "y"); // read the field "y"
	y = (float)lua_tonumber(L, -1); // get the value of "y" from the stack
	lua_pop(L, 1); // pop the value of "y" from the stack
	Vec2 v2(x, y); // create an instance of Vec2 on the C/C++ stack
	lua_pop(L, 1); 
	lua_pop(L, 1); 
	Vec2 result = v1 - v2;

	lua_pushnumber(L, result.m_x);
	lua_pushnumber(L, result.m_y);

	lua_CFunction_Vec2(L);
	return 1;
}
