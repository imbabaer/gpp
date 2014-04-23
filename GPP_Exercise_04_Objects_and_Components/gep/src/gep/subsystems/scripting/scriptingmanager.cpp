#include "stdafx.h"


#include "gepimpl/subsystems/scripting.h"
#include "gep/exception.h"
#include "gep/utils.h"

lua_State* lua::L = nullptr;
using namespace lua;

gep::ScriptingManager::ScriptingManager(const std::string& scriptsRoot, const std::string& importantScriptsRoot) :
    m_scriptsRoot(scriptsRoot),
    m_importantScriptsRoot(importantScriptsRoot),
    m_scriptsToLoad()
{
    // create a Lua state
    L = luaL_newstate();

    // open all standard libraries
    luaL_openlibs(L);
}

gep::ScriptingManager::~ScriptingManager()
{
    lua_close(L);
}

void gep::ScriptingManager::initialize()
{
}

void gep::ScriptingManager::destroy()
{
}

void gep::ScriptingManager::update(float elapsedTime)
{
}

void gep::ScriptingManager::loadScript(const std::string& filename, LoadOptions::Enum loadOptions)
{
    auto scriptFileName = constructFileName(filename, loadOptions);

    // load and execute a Lua file
    int err = luaL_loadfile(L, scriptFileName.c_str());
    if(err != LUA_OK)
    {
        // the top of the stack should be the error string
        if (lua_isstring(L, lua_gettop(L)))
        {
            // get the top of the stack as the error and pop it off
            const char* theError = lua_tostring(L, lua_gettop(L));
            lua_pop(L, 1);

            //luaL_error(L, theError);
            throw ScriptLoadException(theError);
        }
        else
        {
            //luaL_error(L, "Unknown error loading Lua file \"%s\"", filename);
            throw ScriptLoadException(format("Unknown error loading Lua file \"%s\"", scriptFileName.c_str()));
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

                //luaL_error(L, theError);
                throw ScriptExecutionException(theError);
            }
            else
            {
                //luaL_error(L, "Unknown error executing Lua file \"%s\"", filename);
                throw ScriptExecutionException(format("Unknown error executing Lua file \"%s\"", scriptFileName.c_str()));
            }
        }
    }
}

void gep::ScriptingManager::registerScript(const std::string& filename, LoadOptions::Enum loadOptions)
{
    m_scriptsToLoad.append(constructFileName(filename, loadOptions));
}

void gep::ScriptingManager::loadAllRegisteredScripts()
{
    for (auto& scriptFileName : m_scriptsToLoad)
    {
        loadScript(scriptFileName, LoadOptions::None);
    }
}

std::string gep::ScriptingManager::constructFileName(const std::string& filename, LoadOptions::Enum loadOptions)
{
    if (loadOptions == LoadOptions::None)
    {
        return filename;
    }

    std::stringstream completeName;
    switch (loadOptions)
    {
    case LoadOptions::PathIsAbsolute:
        completeName << filename;
        break;
    case LoadOptions::PathIsRelative:
        completeName << m_scriptsRoot << filename;
        break;
    case LoadOptions::IsImportantScript:
        completeName << m_importantScriptsRoot << filename;
        break;
    default:
        GEP_ASSERT(false, "Invalid script loading option!", loadOptions);
        break;
    }
    return completeName.str();
}


void gep::ScriptingManager::bindEnum(const char* enumName, ...)
{
    lua_newtable(lua::L);
    const char* ename;
    int         evalue;
    va_list args;
    va_start(args, enumName);
    while ((ename = va_arg(args, const char*)) != 0)
    {
        evalue = va_arg(args, int);
        lua::push(ename);
        lua::push(evalue);
        lua_settable(lua::L, -3);
    }
    va_end(args);
    lua_setglobal(lua::L, enumName);
}
