#pragma once
#include "Vec2.h"

class Player
{
public:
	static Player*		getInstance();
	static void			destroy();

	const char*			getName() const;
	void				setName(const std::string& name);

	Vec2				getPosition() const;
	void				setPosition(const Vec2& position);

	static void			lua_bind(lua_State* L);

	static int			lua_CFunction_getName(lua_State* L);
	static int			lua_CFunction_setName(lua_State* L);

	static int			lua_CFunction_getPosition(lua_State* L);
	static int			lua_CFunction_setPosition(lua_State* L);

private:
	Player(){}						// not constructible
	Player(const Player& other) {}	// not copyable
	Player(Player&& other) {}		// not moveable

	static Player*		s_pInstance;
	std::string			m_name;
	Vec2				m_position;
};
