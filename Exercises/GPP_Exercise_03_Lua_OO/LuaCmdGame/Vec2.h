#pragma once

struct Vec2
{

	friend class Player;
	
	Vec2();
	Vec2(float x, float y);
	
	float				length() const;

	Vec2				operator+(const Vec2& other) const;
	Vec2				operator-(const Vec2& other) const;

	static void			lua_bind(lua_State* L);

	static int			lua_CFunction_Vec2(lua_State* L);

	static int			lua_CFunction_length(lua_State* L);

	static int			lua_CFunction_add(lua_State* L);
	static int			lua_CFunction_sub(lua_State* L);

	float				m_x, m_y;	// public members
};
