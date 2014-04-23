#pragma once

#include "gep/interfaces/subsystem.h"
#include "gep/interfaces/scripting/luaHelper.h"
#include "gep/container/hashmap.h"

#include <assert.h>
#include <string>
#include <map>
#include <functional>

namespace gep
{
    // TODO inheritance
    // TODO update calls
    // TODO events
    // TODO error recovery with reasonable messages
    // TODO loading scripts from lua
    // TODO resource type and hot reloading of scripts

    class GEP_API IScriptingManager : public ISubsystem
    {
    public:

        struct LoadOptions 
        {
            enum Enum
            {
                None = -1,
                PathIsRelative,
                PathIsAbsolute,
                IsImportantScript,

                Default = PathIsRelative,
            };
        };

        IScriptingManager() {}
        virtual ~IScriptingManager() = 0 {}

        virtual void loadScript(const std::string& filename, LoadOptions::Enum loadOptions = LoadOptions::Default) = 0;

        /// \brief Used to register scripts that will be loaded as regular user scripts.
        ///        Should be called in an initialization script (e.g. initialize.lua)
        virtual void registerScript(const std::string& filename, LoadOptions::Enum loadOptions = LoadOptions::Default) = 0;

        /// \brief Loads all scripts previously registered with registerScript(...).
        /// \remarks May throw a ScriptException (ScriptLoadException, ScriptExecutionException)
        virtual void loadAllRegisteredScripts() = 0;

        virtual       std::string& getScriptsRoot()       = 0;
        virtual const std::string& getScriptsRoot() const = 0;
        virtual void setScriptsRoot(const std::string& value) = 0;

        virtual       std::string& getImportantScriptsRoot()       = 0;
        virtual const std::string& getImportantScriptsRoot() const = 0;
        virtual void setImportantScriptsRoot(const std::string& value) = 0;
        
        template <typename T>
        void bind(const char* className, T* instance = nullptr)
        {
            T::Lua_Bind<T>(className);
            if (instance != nullptr)
            {
                addGlobalInstance<T>(instance);
            }
        }

        virtual void bindEnum(const char* enumName, ...) = 0;

#if _MSC_VER >= 1800
        template<typename R = void>
        R callFunction(const char* n)
        {
            callFunctionBegin(n);
            lua_call(lua::L, 0, lua::countReturnValues<R>());
            return callFunctionEnd<R>();
        }

        template<typename R = void, typename... Args>
        R callFunction(const char* n, Args&&... args)
        {
            callFunctionBegin(n);
            std::tuple<Args...> t(args...);
            tuple::for_each_in_tuple(t, tuple::push_functor());
            lua_call(lua::L, sizeof...(Args), lua::countReturnValues<R>());
            return callFunctionEnd<R>();
        }
#else
        void callFunction(const char* n)
        { callFunctionBegin(n); lua_call(lua::L, 0, lua::countReturnValues<void>()); return callFunctionEnd<void>(); }

        template <typename R> R callFunction(const char* n)
        { callFunctionBegin(n); lua_call(lua::L, 0, lua::countReturnValues<R>()); return callFunctionEnd<R>(); }

        template<typename R, typename P0> R callFunction(const char* n, P0 p0)
        { callFunctionBegin(n); lua::push(p0); lua_call(lua::L, 1, lua::countReturnValues<R>()); return callFunctionEnd<R>(); }

        template<typename R, typename P0, typename P1> R callFunction(const char* n, P0 p0, P1 p1)
        { callFunctionBegin(n); lua::push(p0); lua::push(p1); lua_call(lua::L, 2, lua::countReturnValues<R>()); return callFunctionEnd<R>(); }

        template<typename R, typename P0, typename P1, typename P2> R callFunction(const char* n, P0 p0, P1 p1, P2 p2)
        { callFunctionBegin(n); lua::push(p0); lua::push(p1); lua::push(p2); lua_call(lua::L, 3, lua::countReturnValues<R>()); return callFunctionEnd<R>(); }

        template<typename R, typename P0, typename P1, typename P2, typename P3> R callFunction(const char* n, P0 p0, P1 p1, P2 p2, P3 p3)
        { callFunctionBegin(n); lua::push(p0); lua::push(p1); lua::push(p2); lua::push(p3); lua_call(lua::L, 4, lua::countReturnValues<R>()); return callFunctionEnd<R>(); }

#endif

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(registerScript, "_registerScript")
        LUA_BIND_REFERENCE_TYPE_END

    private:
        void callFunctionBegin(const char* n)
        {
            lua_getglobal(lua::L, n);
            if (lua_isnil(lua::L, -1))
                luaL_error(lua::L, "function \"%s\" was not found", n);
        }
        template <typename R>
        R callFunctionEnd()
        {
            if (lua::countReturnValues<R>() == 1)
                return lua::pop<R>(-1);
            return R();
        }

        template <typename T>
        void addGlobalInstance(T* __ptr)
        {
            lua_newtable(lua::L);
            lua::pushTableEntry<const char*, void*>("__ptr", __ptr);
            luaL_setmetatable(lua::L, ScriptTypeInfo<T>::instance().getMetaTableName().c_str());
            lua_setglobal(lua::L, ScriptTypeInfo<T>::instance().getClassName().c_str());
        }
    };
    
    template<typename T>
    class ScriptTypeInfo
    {
        std::string m_className;
        std::string m_MetaTableName;
    public:
        inline static ScriptTypeInfo<T>& instance()
        {
            static ScriptTypeInfo<T> info;
            return info;
        }
        inline void setClassName(const std::string& name) { m_className = name; m_MetaTableName = name + "_Meta"; }
        inline const std::string& getClassName() const { return m_className; }
        inline const std::string& getMetaTableName() const { return m_MetaTableName; }
    };
    
}
