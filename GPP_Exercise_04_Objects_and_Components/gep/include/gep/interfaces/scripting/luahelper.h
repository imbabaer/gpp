#pragma once

#include <tuple>
#include <string>
#include "gep/utils.h"
#include "lua.hpp"

#define __RM_P(T)      std::remove_pointer<T>::type
#define __RM_C(T)      std::remove_const<T>::type
#define __RM_C_R(T)    std::remove_const<std::remove_reference<T>::type>::type

/// \source
///  http://stackoverflow.com/questions/16387354/template-tuple-calling-a-function-on-each-element 
///
#if _MSC_VER >= 1800
namespace tuple
{
    template<int... Is>
    struct seq { };

    template<int N, int... Is>
    struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

    template<int... Is>
    struct gen_seq<0, Is...> : seq<Is...> { };

    template<typename T, typename F, int... Is>
    void for_each(T&& t, F f, seq<Is...>)
    {
        auto l = { (f(std::get<Is>(t)), 0)... };
    }

    template<typename... Ts, typename F>
    void for_each_in_tuple(std::tuple<Ts...> const& t, F f)
    {
        for_each(t, f, gen_seq<sizeof...(Ts)>());
    }

    struct push_functor
    {
        template <typename T>
        void operator () (T t)
        {
            lua::push<T>(t);
        }
    };
}
#endif

namespace lua
{
    extern GEP_API lua_State* L;

    namespace structs
    {
        template <typename T>
        struct typeHandling {};

#define _LUA_NUMBER_TYPE_HANDLING_HELPER(type, pusher, popper) template <>                          \
    struct typeHandling<type>                                                                       \
    {                                                                                               \
        static int push(type value)                                                                 \
        {                                                                                           \
            pusher(L, static_cast<decltype(popper(L, 0))>(value));                                  \
            return 1;                                                                               \
        }                                                                                           \
        static type pop(int idx)                                                                    \
        {                                                                                           \
            auto typeId = lua_type(L, idx);                                                         \
            if (typeId != LUA_TNUMBER) {                                                            \
                auto typeName = lua_typename(L, typeId);                                            \
                luaL_error(L, gep::format("Expected \"" #type "\", got \"%s\"", typeName).c_str()); \
            }                                                                                       \
            auto value = popper(L, idx);                                                            \
            return type(value);                                                                     \
        }                                                                                           \
    };
        _LUA_NUMBER_TYPE_HANDLING_HELPER(gep::int8,  lua_pushinteger, lua_tointeger);
        _LUA_NUMBER_TYPE_HANDLING_HELPER(gep::int16, lua_pushinteger, lua_tointeger);
        _LUA_NUMBER_TYPE_HANDLING_HELPER(gep::int32, lua_pushinteger, lua_tointeger);
        _LUA_NUMBER_TYPE_HANDLING_HELPER(gep::int64, lua_pushinteger, lua_tointeger);

        _LUA_NUMBER_TYPE_HANDLING_HELPER(gep::uint8,  lua_pushunsigned, lua_tounsigned);
        _LUA_NUMBER_TYPE_HANDLING_HELPER(gep::uint16, lua_pushunsigned, lua_tounsigned);
        _LUA_NUMBER_TYPE_HANDLING_HELPER(gep::uint32, lua_pushunsigned, lua_tounsigned);
        _LUA_NUMBER_TYPE_HANDLING_HELPER(gep::uint64, lua_pushunsigned, lua_tounsigned);

        _LUA_NUMBER_TYPE_HANDLING_HELPER(float, lua_pushnumber, lua_tonumber);
        _LUA_NUMBER_TYPE_HANDLING_HELPER(double, lua_pushnumber, lua_tonumber);

        template <>
        struct typeHandling<bool>
        {
            static int push(bool value)
            {
                lua_pushboolean(L, value ? 1 : 0);
                return 1;
            }
            static bool pop(int idx)
            {
                if (!lua_isboolean(L, idx))
                {
                    auto typeId = lua_type(L, idx);
                    auto typeName = lua_typename(L, typeId);
                    luaL_error(L, gep::format("Expected \"boolean\", got \"%s\"", typeName).c_str()); 
                }
                auto value = lua_toboolean(L, idx) != 0;
                return value;
            }
        };

        template <>
        struct typeHandling<const char*>
        {
            static int push(const char* value)
            {
                lua_pushstring(L, value);
                return 1;
            }
            static const char* pop(int idx)
            {
                if (!lua_isstring(L, idx))
                {
                    auto typeId = lua_type(L, idx);
                    auto typeName = lua_typename(L, typeId);
                    luaL_error(L, gep::format("Expected \"const char*\", got \"%s\"", typeName).c_str()); 
                }
                auto value = lua_tostring(L, idx);
                return value;
            }
        };

        template <>
        struct typeHandling<std::string>
        {
            static int push(const std::string& value)
            {
                lua_pushstring(L, value.c_str());
                return 1;
            }
            static std::string pop(int idx)
            {
                if (!lua_isstring(L, idx))
                {
                    auto typeId = lua_type(L, idx);
                    auto typeName = lua_typename(L, typeId);
                    luaL_error(L, gep::format("Expected \"const char*\", got \"%s\"", typeName).c_str()); 
                }
                auto value = lua_tostring(L, idx);
                return value;
            }
        };

        template <>
        struct typeHandling<void*>
        {
            static int push(void* pointer)
            {
                lua_pushlightuserdata(L, pointer);
                return 1;
            }
            static void* pop(int idx)
            {
                if (!lua_islightuserdata(L, idx))
                {
                    auto typeId = lua_type(L, idx);
                    auto typeName = lua_typename(L, typeId);
                    luaL_error(L, gep::format("Expected \"void*\", got \"%s\"", typeName).c_str()); 
                }
                void* value = const_cast<void*>(lua_topointer(L, idx));
                return value;
            }
        };

        template <>
        struct typeHandling<void>
        {
            static int push(void)
            {
                return 0;
            }
            static void pop(int)
            {
                return;
            }
        };

        template <typename T, bool is_type>
        struct enum_or_typeHandling { };

        template <typename T>
        struct enum_or_typeHandling<T, false>
        {
            static int push(T t)
            {
                return typeHandling<T>::push(t);
            }
            static T pop(int idx)
            {
                return typeHandling<T>::pop(idx);
            }
        };

        template <typename T>
        struct enum_or_typeHandling<T, true>    // T is an enum
        {
            static int push(T t)
            {
                return typeHandling<int>::push(t);
            }
            static T pop(int idx)
            {
                return (T)typeHandling<int>::pop(idx);
            }
        };

        struct TypeMarker { };
        struct ReferenceTypeMarker : TypeMarker { };
        struct ValueTypeMarker : TypeMarker { };

        template <typename T>
        struct objectHandling { };

        template <>
        struct objectHandling<ReferenceTypeMarker>
        {
            template <typename U>
            static U pop(int idx)
            {
                lua_getfield(L, idx, "__ptr");
                void* __ptr = typeHandling<void*>::pop(-1);
                lua_pop(L, 1);
                return reinterpret_cast<U>(__ptr);
            }

            template <typename U>
            static int push(U pObject)
            {
                lua_newtable(L);
                pushTableEntry<const char*, void*>("__ptr", pObject);
                //luaL_setmetatable(lua::L, gep::GlobalManager::instance().getScriptingManager()->getMetaTableName<__RM_P(U)>().c_str());
                luaL_setmetatable(lua::L, gep::ScriptTypeInfo<__RM_P(U)>::instance().getMetaTableName().c_str());
                return 1;
            }
        };

        template <>
        struct objectHandling<ValueTypeMarker>
        {
            template <typename U>
            static U pop(int idx)
            {
                U object;
                popValueType(object, idx);
                return object;
            }

            template <typename U>
            static int push(U& object)
            {
                pushValueType(object);
                return 1;
            }
        };

        template <typename T, bool is_object>
        struct object_or_typeHandling { };

        template <typename T>
        struct object_or_typeHandling<T, false>
        {
            static int push(T t)
            {
                return enum_or_typeHandling<T, std::is_enum<T>::value>::push(t);
            }
            static T pop(int idx)
            {
                return enum_or_typeHandling<T, std::is_enum<T>::value>::pop(idx);
            }
        };

        template <typename T>
        struct object_or_typeHandling<T, true>
        {
            static int push(T t)
            {
                return objectHandling<typename __RM_P(T)::LuaType>::push<T>(t);
            }
            static T pop(int idx)
            {
                return objectHandling<typename __RM_P(T)::LuaType>::pop<T>(idx);
            }
        };

        template <>
        struct object_or_typeHandling<void, false>
        {
            static int push(void)
            {
                return 0;
            }
            static void pop(int)
            {
            }
        };

        template <typename T>
        struct referenceType_or_valueTypeHandling { };

        template <>
        struct referenceType_or_valueTypeHandling<ReferenceTypeMarker>
        {
            template <typename U>
            static U& pop(int idx)
            {
                return *objectHandling<ReferenceTypeMarker>::pop<U*>(idx);
            }
        };

        template <>
        struct referenceType_or_valueTypeHandling<ValueTypeMarker>
        {
            template <typename U>
            static U pop(int idx)
            {
                return objectHandling<ValueTypeMarker>::pop<U>(idx);
            }
        };

#if _MSC_VER >= 1800
        template <typename T, typename R, typename... Args>
        struct bindHelper { };
#else
        template <typename T, typename R, typename A1 = void, typename A2 = void, typename A3 = void, typename A4 = void, typename A5 = void>
        struct bindHelper { };
#endif

        template <typename T>
        struct bindHelper<T, void>
        {
            template<void(T::*F)()>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)()>
            static int lua_CFunction(lua_State* L) { (referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(); return 0; }
        };

        template <typename T, typename R>
        struct bindHelper<T, R>
        {
            template<R(T::*F)()>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)()>
            static int lua_CFunction(lua_State* L) { push<R>((referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)()); return 1; }
        };

        template <typename T, typename A1>
        struct bindHelper<T, void, A1>
        {
            template<void(T::*F)(A1)>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1)>
            static int lua_CFunction(lua_State* L) { (referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2)); return 0; }
        };

        template <typename T, typename R, typename A1>
        struct bindHelper<T, R, A1>
        {
            template<R(T::*F)(A1)>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1)>
            static int lua_CFunction(lua_State* L) { push<R>((referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2))); return 1; }
        };

        template <typename T, typename A1, typename A2>
        struct bindHelper<T, void, A1, A2>
        {
            template<void(T::*F)(A1, A2)>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2)>
            static int lua_CFunction(lua_State* L) { (referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3)); return 0; }
        };

        template <typename T, typename R, typename A1, typename A2>
        struct bindHelper<T, R, A1, A2>
        {
            template<R(T::*F)(A1, A2)>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2)>
            static int lua_CFunction(lua_State* L) { push<R>((referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3))); return 1; }
        };

        template <typename T, typename A1, typename A2, typename A3>
        struct bindHelper<T, void, A1, A2, A3>
        {
            template<void(T::*F)(A1, A2, A3)>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2, A3)>
            static int lua_CFunction(lua_State* L) { (referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3), pop<__RM_C_R(A3)>(4)); return 0; }
        };

        template <typename T, typename R, typename A1, typename A2, typename A3>
        struct bindHelper<T, R, A1, A2, A3>
        {
            template<R(T::*F)(A1, A2, A3)>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2, A3)>
            static int lua_CFunction(lua_State* L) { push<R>((referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3), pop<__RM_C_R(A3)>(4))); return 1; }
        };

        template <typename T, typename A1, typename A2, typename A3, typename A4>
        struct bindHelper<T, void, A1, A2, A3, A4>
        {
            template<void(T::*F)(A1, A2, A3, A4)>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2, A3, A4)>
            static int lua_CFunction(lua_State* L) { (referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3), pop<__RM_C_R(A3)>(4), pop<__RM_C_R(A4)>(5)); return 0; }
        };

        template <typename T, typename R, typename A1, typename A2, typename A3, typename A4>
        struct bindHelper<T, R, A1, A2, A3, A4>
        {
            template<R(T::*F)(A1, A2, A3, A4)>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2, A3, A4)>
            static int lua_CFunction(lua_State* L) { push<R >> ((referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3), pop<__RM_C_R(A3)>(4), pop<__RM_C_R(A4)>(5))); return 1; }
        };

#if _MSC_VER >= 1800
        template <typename T, typename R, typename... Args>
        struct bindHelperConst { };
#else
        template <typename T, typename R, typename A1 = void, typename A2 = void, typename A3 = void, typename A4 = void, typename A5 = void>
        struct bindHelperConst { };
#endif

        template <typename T>
        struct bindHelperConst<T, void>
        {
            template<void(T::*F)() const>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)() const>
            static int lua_CFunction(lua_State* L) { (referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(); return 0; }
        };

        template <typename T, typename R>
        struct bindHelperConst<T, R>
        {
            template<R(T::*F)() const>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)() const>
            static int lua_CFunction(lua_State* L) { push<R>((referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)()); return 1; }
        };

        template <typename T, typename A1>
        struct bindHelperConst<T, void, A1>
        {
            template<void(T::*F)(A1) const>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1) const>
            static int lua_CFunction(lua_State* L) { (referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2)); return 0; }
        };

        template <typename T, typename R, typename A1>
        struct bindHelperConst<T, R, A1>
        {
            template<R(T::*F)(A1) const>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1) const>
            static int lua_CFunction(lua_State* L) { push<R>((referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2))); return 1; }
        };

        template <typename T, typename A1, typename A2>
        struct bindHelperConst<T, void, A1, A2>
        {
            template<void(T::*F)(A1, A2) const>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2) const>
            static int lua_CFunction(lua_State* L) { (referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3)); return 0; }
        };
            
        template <typename T, typename R, typename A1, typename A2>
        struct bindHelperConst<T, R, A1, A2>
        {
            template<R(T::*F)(A1, A2) const>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2) const>
            static int lua_CFunction(lua_State* L) { push<R>((referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3))); return 1; }
        };

        template <typename T, typename A1, typename A2, typename A3>
        struct bindHelperConst<T, void, A1, A2, A3>
        {
            template<void(T::*F)(A1, A2, A3) const>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2, A3) const>
            static int lua_CFunction(lua_State* L) { (referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3), pop<__RM_C_R(A3)>(4)); return 0; }
        };

        template <typename T, typename R, typename A1, typename A2, typename A3>
        struct bindHelperConst<T, R, A1, A2, A3>
        {
            template<R(T::*F)(A1, A2, A3) const>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2, A3) const>
            static int lua_CFunction(lua_State* L) { push<R>((referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3), pop<__RM_C_R(A3)>(4))); return 1; }
        };

        template <typename T, typename A1, typename A2, typename A3, typename A4>
        struct bindHelperConst<T, void, A1, A2, A3, A4>
        {
            template<void(T::*F)(A1, A2, A3, A4) const>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2, A3, A4) const>
            static int lua_CFunction(lua_State* L) { (referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3), pop<__RM_C_R(A3)>(4), pop<__RM_C_R(A4)>(5)); return 0; }
        };

        template <typename T, typename R, typename A1, typename A2, typename A3, typename A4>
        struct bindHelperConst<T, R, A1, A2, A3, A4>
        {
            template<R(T::*F)(A1, A2, A3, A4) const>
            void bind(const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2, A3, A4) const>
            static int lua_CFunction(lua_State* L) { push<R>((referenceType_or_valueTypeHandling<typename T::LuaType>::pop<T>(1).*F)(pop<__RM_C_R(A1)>(2), pop<__RM_C_R(A2)>(3), pop<__RM_C_R(A3)>(4), pop<__RM_C_R(A4)>(5))); return 1; }
        };

    } // structs

    template <typename T>
    int push(T t)
    {
        return structs::object_or_typeHandling<__RM_C(T), std::is_class<__RM_P(T)>::value && !std::is_same<__RM_C_R(T), std::string>::value>::push((__RM_C(T))t);
    }

    template <typename T>
    T pop(int idx)
    {
        return structs::object_or_typeHandling<T, std::is_class<__RM_P(T)>::value && !std::is_same<__RM_C_R(T), std::string>::value>::pop(idx);
    }
    
    template <typename T>
    int countReturnValues()
    {
        return std::is_void<T>::value ? 0 : 1;
    }

    template <typename K, typename V>
    int pushTableEntry(K k, V v)
    {
        push<K>(k);
        push<V>(v);
        lua_settable(L, -3);
        return 1;
    }

    template <typename T>
    int pushValueType(T& t)
    {
        lua_newtable(L);
        t.Lua_TableValueType<T>(true, 0);
        return 1;
    }

    template <typename T>
    void popValueType(T& t, int idx)
    {
        bool isTable = lua_istable(L, idx);
        t.Lua_TableValueType<T>(false, idx, !isTable);
    }

    template <typename T, typename R>
    static structs::bindHelper<T, R> bind(R(T::*func)()) { return structs::bindHelper<T, R>(); }

    template <typename T, typename R, typename A1>
    static structs::bindHelper<T, R, A1> bind(R(T::*func)(A1)) { return structs::bindHelper<T, R, A1>(); }

    template <typename T, typename R, typename A1, typename A2>
    static structs::bindHelper<T, R, A1, A2> bind(R(T::*func)(A1, A2)) { return structs::bindHelper<T, R, A1, A2>(); }

    template <typename T, typename R, typename A1, typename A2, typename A3>
    static structs::bindHelper<T, R, A1, A2, A3> bind(R(T::*func)(A1, A2, A3)) { return structs::bindHelper<T, R, A1, A2, A3>(); }

    template <typename T, typename R, typename A1, typename A2, typename A3, typename A4>
    static structs::bindHelper<T, R, A1, A2, A3, A4> bind(R(T::*func)(A1, A2, A3, A4)) { return structs::bindHelper<T, R, A1, A2, A3, A4>(); }

    template <typename T, typename R>
    static structs::bindHelperConst<T, R> bind(R(T::*func)() const) { return structs::bindHelperConst<T, R>(); }

    template <typename T, typename R, typename A1>
    static structs::bindHelperConst<T, R, A1> bind(R(T::*func)(A1) const) { return structs::bindHelperConst<T, R, A1>(); }

    template <typename T, typename R, typename A1, typename A2>
    static structs::bindHelperConst<T, R, A1, A2> bind(R(T::*func)(A1, A2) const) { return structs::bindHelperConst<T, R, A1, A2>(); }

    template <typename T, typename R, typename A1, typename A2, typename A3>
    static structs::bindHelperConst<T, R, A1, A2, A3> bind(R(T::*func)(A1, A2, A3) const) { return structs::bindHelperConst<T, R, A1, A2, A3>(); }

    template <typename T, typename R, typename A1, typename A2, typename A3, typename A4>
    static structs::bindHelperConst<T, R, A1, A2, A3, A4> bind(R(T::*func)(A1, A2, A3, A4) const) { return structs::bindHelperConst<T, R, A1, A2, A3, A4>(); }
}

// You should not use this macro directly!
#define _LUA_BIND_TYPE_BEGIN public:                                                                \
    template <typename __T>                                                                         \
    static void Lua_Bind(const std::string& className)                                              \
    {                                                                                               \
        /* remember the className */                                                                \
        gep::ScriptTypeInfo<__T>::instance().setClassName(className);                               \
        /* create new metatable and store it in the registry */                                     \
        luaL_newmetatable(lua::L, gep::ScriptTypeInfo<__T>::instance().getMetaTableName().c_str()); \
                                                                                                    \
        /* TODO inheritance */                                                                      \
        /* Class_Sub -> setmetatable(Class_Sub_Meta, Class_Meta) */                                 \
                                                                                                    \
        /* add __index to metatable*/                                                               \
        lua_pushvalue(lua::L, -1);                                                                  \
        lua_setfield(lua::L, -2, "__index");

#define LUA_BIND_FUNCTION(function)                  \
    lua::bind(&function).bind<&function>(#function);

#define LUA_BIND_FUNCTION_NAMED(function, name) \
    lua::bind(&function).bind<&function>(name);

#define LUA_BIND_REFERENCE_TYPE_BEGIN public:          \
    typedef lua::structs::ReferenceTypeMarker LuaType; \
    _LUA_BIND_TYPE_BEGIN

#define LUA_BIND_REFERENCE_TYPE_END \
    }

#define LUA_BIND_VALUE_TYPE_BEGIN public:          \
    typedef lua::structs::ValueTypeMarker LuaType; \
    _LUA_BIND_TYPE_BEGIN

#define LUA_BIND_VALUE_TYPE_MEMBERS                                                         \
        /* add "create" function to metatable */                                            \
        lua_pushcfunction(lua::L, Lua_Create<__T>);                                         \
        /* local t = T(params) OR local t = T({params}) */                                  \
        lua_setglobal(lua::L, gep::ScriptTypeInfo<__T>::instance().getClassName().c_str()); \
    }                                                                                       \
                                                                                            \
    template <typename __T>                                                                 \
    static int Lua_Create(lua_State* L)                                                     \
    {                                                                                       \
        __T t;                                                                              \
        /* get constructor parameters from stack */                                         \
        if (lua_gettop(L) >= 1)                                                             \
            lua::popValueType(t, 1);                                                        \
        lua::pushValueType(t);                                                              \
        return 1;                                                                           \
    }                                                                                       \
                                                                                            \
    template <typename __T>                                                                 \
    void Lua_TableValueType(bool push, int idx, bool popParams = false)                     \
    {

#define LUA_BIND_MEMBER_NAMED(memberVariable, luaName)                      \
        if (push) {                                                         \
            lua::pushTableEntry<const char*, decltype(memberVariable)>(     \
                luaName, memberVariable);                                   \
        }                                                                   \
        else {                                                              \
            if (popParams) {                                                \
                memberVariable = lua::pop<decltype(memberVariable)>(idx++); \
            }                                                               \
            else {                                                          \
                lua_getfield(lua::L, idx, luaName);                         \
                memberVariable = lua::pop<decltype(memberVariable)>(-1);    \
                lua_pop(lua::L, 1);                                         \
            }                                                               \
        }

#define LUA_BIND_MEMBER(memberVariable) LUA_BIND_MEMBER_NAMED(memberVariable, #memberVariable)

#define LUA_BIND_VALUE_TYPE_END                                                                               \
        if (push) luaL_setmetatable(lua::L, gep::ScriptTypeInfo<__T>::instance().getMetaTableName().c_str()); \
    }
