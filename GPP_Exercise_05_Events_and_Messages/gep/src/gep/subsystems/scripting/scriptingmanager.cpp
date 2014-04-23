#include "stdafx.h"


#include "gepimpl/subsystems/scripting.h"
#include "gep/exception.h"
#include "gep/utils.h"
#include "gep/memory/allocator.h"
#include "gep/memory/leakDetection.h"

lua_State* lua::L = nullptr;

namespace gep
{
    void* scriptAllocator(void* userData, void* ptr, size_t originalSize, size_t newSize)
    {
        GEP_ASSERT(userData != nullptr, "Lua allocation called with invalid user data!");
        auto& allocator = *reinterpret_cast<IAllocator*>(userData);

        if (newSize == 0) // We are supposed to free the pointer.
        {
            allocator.freeMemory(ptr);
            return nullptr;
        }

        if (ptr == nullptr) // we are supposed to allocate new memory.
        {
            return allocator.allocateMemory(newSize);
        }

        auto newPtr = allocator.allocateMemory(newSize);
        GEP_ASSERT(newPtr, "Out of memory.");

        auto size = GEP_MIN(originalSize, newSize);
        GEP_ASSERT(size <= newSize);
        GEP_ASSERT(size <= originalSize);

        memcpy(newPtr, ptr, size);
        allocator.freeMemory(ptr);

        return newPtr;
    }
}

gep::ScriptingManager::ScriptingManager(const std::string& scriptsRoot, const std::string& importantScriptsRoot) :
    m_pAllocator(&g_stdAllocator),
    m_state(State::NotAcceptingScriptRegistration),
    m_scriptsRoot(scriptsRoot),
    m_importantScriptsRoot(importantScriptsRoot),
    m_scriptsToLoad()
{
    // create a Lua state with a custom allocator
    lua::L = lua_newstate(&gep::scriptAllocator, m_pAllocator);

    // open all standard libraries
    luaL_openlibs(lua::L);
}

gep::ScriptingManager::~ScriptingManager()
{
    lua_close(lua::L);
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
    int err = luaL_loadfile(lua::L, scriptFileName.c_str());
    if(err != LUA_OK)
    {
        // the top of the stack should be the error string
        if (lua_isstring(lua::L, lua_gettop(lua::L)))
        {
            // get the top of the stack as the error and pop it off
            const char* theError = lua_tostring(lua::L, lua_gettop(lua::L));
            lua_pop(lua::L, 1);

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
        err = lua_pcall(lua::L, 0, LUA_MULTRET, 0);
        if (err != LUA_OK)
        {
            // the top of the stack should be the error string
            if (lua_isstring(lua::L, lua_gettop(lua::L)))
            {
                // get the top of the stack as the error and pop it off
                const char* theError = lua_tostring(lua::L, lua_gettop(lua::L));
                lua_pop(lua::L, 1);

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
    GEP_ASSERT(m_state == State::AcceptingScriptRegistration,
        "You are not allowed to register a script at this point! "
        "Check 'filename' below to see which script was supposed to be registered",
        filename);
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

gep::int32 gep::ScriptingManager::memoryUsed() const
{
    return lua_gc(lua::L, LUA_GCCOUNT, 0);
}
