#include "stdafx.h"
#include "gep/interfaces/scripting/luaHelper.h"

namespace helper
{
    void indent(const size_t level, std::ostringstream& output)
    {
        for (size_t i = 0; i < level; i++)
        {
            output << "  ";
        }
    }
}

void lua::utils::printType(TraversalInfo input)
{
    auto luaType = lua_type(input.L, input.index);
    auto luaTypeName = lua_typename(input.L, luaType);

    if (input.indent)
    {
        helper::indent(input.level, input.output);
    }

    input.output << luaTypeName;
    switch (luaType)
    {
    case LUA_TNIL:
        break;
    case LUA_TBOOLEAN:
        input.output << '(' << std::boolalpha << lua_toboolean(input.L, input.index) << ')';
        break;
    case LUA_TNUMBER:
        input.output << '(' << lua_tonumber(input.L, input.index) << ')';
        break;
    case LUA_TSTRING:
        input.output << "(\"" << lua_tostring(input.L, input.index) << "\")";
        break;
    case LUA_TTABLE:
        if (input.level < input.maxLevel)
        {
            dumpTable(input);
        }
        else
        {
            input.output << "{<max-level-reached>}";
        }
        break;
    case LUA_TFUNCTION:
    case LUA_TLIGHTUSERDATA:
    case LUA_TUSERDATA:
    case LUA_TTHREAD:
        input.output << "(?)";
        break;
    default:
        GEP_ASSERT(false, "Unexpected lua type", luaType, luaTypeName);
        break;
    }
}

void lua::utils::dumpTable(TraversalInfo input)
{
    bool tableIsNotEmpty = false;
    input.output << '{';

    const int top = lua_gettop(L);

    const int tableIndex = input.index;

    TraversalInfo keyInput(input);
    keyInput.index = top + 1;
    keyInput.level += 1;
    keyInput.indent = true;

    TraversalInfo valueInput(input);
    valueInput.index = top + 2;
    valueInput.level += 1;
    valueInput.indent = false;

    // first 'dummy' key
    lua_pushnil(L);

    // table is in the stack at 'tableIndex'
    while (lua_next(L, tableIndex) != 0)
    {
        tableIsNotEmpty = true;
        input.output << '\n';

        printType(keyInput);
        input.output << " => ";
        printType(valueInput);

        // removes 'value' but keeps 'key' for next iteration
        lua_pop(L, 1);
    }
    if (tableIsNotEmpty)
    {
        input.output << '\n';
        if (input.indent)
        {
            helper::indent(input.level, input.output);
        }
    }
    input.output << '}';
}

std::string lua::utils::dumpStack(lua_State* L)
{
    std::ostringstream output;
    const size_t level = 0;
    const size_t maxLevel = 2;
    int originalTop = lua_gettop(L);

    output << "Stack size = " << originalTop;

    for (int index = originalTop; index > 0; --index)
    {
        output << "\n[" << index << ',' << -1 + index - originalTop  << "] ";
        printType(utils::TraversalInfo(L, index, level, maxLevel, output));
    }
    output << '\n';

    auto currentTop = lua_gettop(L);
    GEP_ASSERT(originalTop == currentTop, "dumpStack popped too many or too few values!", originalTop, currentTop);

    return output.str();
}

//////////////////////////////////////////////////////////////////////////

void lua::FunctionWrapper::initializeTable(int functionIndex)
{
    // create a table that will store the reference count for us
    lua_createtable(m_L, 2, 0);
    auto tableIndex = lua_gettop(m_L);

    // set the ref count within the table to 1
    lua_pushinteger(m_L, s_refCountIndex);
    lua_pushinteger(m_L, 1);
    lua_rawset(m_L, tableIndex);

    // store the function in the table
    lua_pushinteger(m_L, s_functionIndex);
    lua_pushvalue(m_L, functionIndex);
    lua_rawset(m_L, tableIndex);

    // generate a unique reference to our helper table
    m_tableReference = luaL_ref(m_L, LUA_REGISTRYINDEX);
}

void lua::FunctionWrapper::addReference()
{
    // push the table on the stack
    pushTable();
    auto tableIndex = lua_gettop(m_L);

    lua_pushinteger(m_L, s_refCountIndex);
    lua_rawget(m_L, tableIndex);

    auto refCount = lua_tointeger(m_L, -1);
    ++refCount;

    lua_pop(L, 1);

    lua_pushinteger(m_L, s_refCountIndex);
    lua_pushinteger(m_L, refCount);
    lua_rawset(m_L, tableIndex);

    lua_pop(L, 1);
}

void lua::FunctionWrapper::removeReference()
{
    // push the table on the stack
    pushTable();
    auto tableIndex = lua_gettop(m_L);

    lua_pushinteger(m_L, s_refCountIndex);
    lua_rawget(m_L, tableIndex);

    auto refCount = lua_tointeger(m_L, -1);
    --refCount;

    lua_pop(L, 1);

    lua_pushinteger(m_L, s_refCountIndex);
    lua_pushinteger(m_L, refCount);
    lua_rawset(m_L, tableIndex);

    if (refCount == 0)
    {
        luaL_unref(m_L, LUA_REGISTRYINDEX, m_tableReference);
        m_tableReference = s_invalidReference;
    }
    else
    {
        lua_pushinteger(m_L, s_refCountIndex);
        lua_pushinteger(m_L, refCount);
        lua_rawset(m_L, tableIndex);
    }

    lua_pop(L, 1);
}

void lua::FunctionWrapper::push()
{
    // Push the table on the stack
    pushTable();
    auto tableIndex = lua_gettop(m_L);

    lua_pushinteger(m_L, s_functionIndex);
    lua_rawget(m_L, tableIndex);

    // Swap the table with the function so that the table is at the top of the stack.
    lua_insert(m_L, -2);

    // Remove the table from the top of the stack.
    lua_pop(m_L, 1);
}

void lua::FunctionWrapper::pushTable()
{
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableReference);
}