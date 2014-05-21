#pragma once

#include "gep/interfaces/subsystem.h"
#include "gep/scripting/luaHelper.h"
#include "gep/container/hashmap.h"

#include "gep/singleton.h"
#include "gep/exception.h"

namespace gep
{
    // TODO inheritance
    // TODO update calls
    // TODO events
    // TODO error recovery with reasonable messages
    // TODO loading scripts from lua
    // TODO resource type and hot reloading of scripts

    typedef lua::FunctionWrapper ScriptFunctionWrapper;
    typedef lua::TableWrapper ScriptTableWrapper;

    class GEP_API IScriptingManager : public ISubsystem
    {
    public:

        enum class State
        {
            AcceptingScriptRegistration,
            NotAcceptingScriptRegistration
        };

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

        virtual void setState(State state) = 0;
        virtual State getState() const = 0;

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

        // in KB
        virtual int32 memoryUsed() const = 0;
        virtual void collectGarbage() = 0;

        virtual lua_State* getState() = 0;

        virtual void debugBreak(const char* message) const = 0;
        
        template <typename T>
        void bind(const char* className, T* instance = nullptr)
        {
            lua::utils::StackChecker check(getState(), 0);
            T::Lua_Bind<T>(getState(), className);
            if (instance != nullptr)
            {
                lua::utils::StackChecker check(getState(), 0);
                addGlobalInstance<T>(instance);
            }
        }

        virtual void bindEnum(const char* enumName, ...) = 0;

#if GEP_VARIADIC_TEMPLATE_ARGUMENTS_ENABLED
        template<typename R = void>
        R callFunction(const char* n)
        {
            callFunctionBegin(n);
            lua_call(getState(), 0, lua::countReturnValues<R>());
            return callFunctionEnd<R>();
        }

        template<typename R = void, typename... Args>
        R callFunction(const char* n, Args&&... args)
        {
            callFunctionBegin(n);
            std::tuple<Args...> t(args...);
            tuple::for_each_in_tuple(t, tuple::push_functor());
            lua_call(getState(), sizeof...(Args), lua::countReturnValues<R>());
            return callFunctionEnd<R>();
        }
#else
        template<typename T_FuncRef>
        void callFunction(T_FuncRef n)
        { callFunctionBegin(n); actualCall(0, lua::countReturnValues<void>()); return callFunctionEnd<void>(); }

        template <typename R, typename T_FuncRef> R callFunction(T_FuncRef n)
        { callFunctionBegin(n); actualCall(0, lua::countReturnValues<R>()); return callFunctionEnd<R>(); }

        template<typename R, typename P0, typename T_FuncRef> R callFunction(T_FuncRef n, P0 p0)
        { callFunctionBegin(n); lua::push(getState(), p0); actualCall(1, lua::countReturnValues<R>()); return callFunctionEnd<R>(); }

        template<typename R, typename P0, typename P1, typename T_FuncRef> R callFunction(T_FuncRef n, P0 p0, P1 p1)
        { callFunctionBegin(n); lua::push(getState(), p0); lua::push(getState(), p1); actualCall(2, lua::countReturnValues<R>()); return callFunctionEnd<R>(); }

        template<typename R, typename P0, typename P1, typename P2, typename T_FuncRef> R callFunction(T_FuncRef n, P0 p0, P1 p1, P2 p2)
        { callFunctionBegin(n); lua::push(getState(), p0); lua::push(getState(), p1); lua::push(getState(), p2); actualCall(3, lua::countReturnValues<R>()); return callFunctionEnd<R>(); }

        template<typename R, typename P0, typename P1, typename P2, typename P3, typename T_FuncRef> R callFunction(T_FuncRef n, P0 p0, P1 p1, P2 p2, P3 p3)
        { callFunctionBegin(n); lua::push(getState(), p0); lua::push(getState(), p1); lua::push(getState(), p2); lua::push(getState(), p3); actualCall(4, lua::countReturnValues<R>()); return callFunctionEnd<R>(); }

#endif

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(registerScript, "_registerScript")
            LUA_BIND_FUNCTION_NAMED(debugBreak, "_debugBreak")
            LUA_BIND_FUNCTION_NAMED(getScriptsRootCopy, "getScriptsRoot")
            LUA_BIND_FUNCTION_NAMED(getImportantScriptsRootCopy, "getImportantScriptsRoot")
        LUA_BIND_REFERENCE_TYPE_END

    private:
        inline std::string getScriptsRootCopy() { return getScriptsRoot(); }
        inline std::string getImportantScriptsRootCopy() { return getImportantScriptsRoot(); }

        void callFunctionBegin(const char* n)
        {
            auto L = getState();
            lua::utils::StackChecker check(L, 1);
            lua_getglobal(L, n);
            if (lua_isnil(L, -1))
                luaL_error(L, "function \"%s\" was not found", n);
        }
        void callFunctionBegin(ScriptFunctionWrapper& funcRef)
        {
            auto L = getState();
            lua::utils::StackChecker check(L, 1);
            funcRef.push();
            if (lua_isnil(L, -1))
                luaL_error(L, "Invalid function reference.");
        }
        template <typename R>
        R callFunctionEnd()
        {
            if (lua::countReturnValues<R>() >= 1)
            {
                try
                {
                    return lua::pop<R>(getState(), -1);
                }
                catch (ScriptExecutionException& exception)
                {
                    GEP_ASSERT(false, "Error getting return value of function call.", exception.what());
                    throw exception;
                }
            }
            return R();
        }
        void actualCall(int numArgs, int numReturnValues)
        {
            auto L = getState();

            // Account for the function that will be popped
            int expectedStackSize(numReturnValues - numArgs - 1);
            lua::utils::StackCleaner cleaner(L, expectedStackSize);

            lua_call(L, numArgs, numReturnValues);
        }

        template <typename T>
        void addGlobalInstance(T* ptr)
        {
            lua::structs::objectHandling<lua::structs::ReferenceTypeMarker>::push(getState(), ptr);
            lua_setglobal(getState(), ScriptTypeInfo<T>::instance().getClassName());
        }
    };
    
    template<typename T>
    class ScriptTypeInfo
    {
        char m_className[64];
        char m_metaTableName[64];
    public:
        inline static ScriptTypeInfo<T>& instance()
        {
            static ScriptTypeInfo<T> info;
            return info;
        }

        inline void setClassName(const char* name)
        {
            strcpy_s(m_className, name);

            strcpy_s(m_metaTableName, name);
            strcat_s(m_metaTableName, "_Meta");
        }
        inline const char* getClassName() const { return m_className; }
        inline const char* getMetaTableName() const { return m_metaTableName; }
    };

    // Inspired by http://en.wikibooks.org/wiki/More_C++_Idioms/Member_Detector
    template<typename T>
    class IsBoundToScript
    {
        typedef char YesType[1];
        typedef char NoType[2];

        struct Fallback { int LuaType; };
        struct Derived : T, Fallback { };

        template<typename U>
        static NoType& test( decltype(U::LuaType)* );
        template<typename U>
        static YesType& test( U* );

    public:
        static const bool value = std::is_scalar<T>::value || sizeof(test<Derived>(nullptr)) == sizeof(YesType);
    };

    template<>
    class IsBoundToScript<lua::FunctionWrapper>
    {
    public:
        static const bool value = true;
    };

    template<>
    class IsBoundToScript<lua::TableWrapper>
    {
    public:
        static const bool value = true;
    };
}
