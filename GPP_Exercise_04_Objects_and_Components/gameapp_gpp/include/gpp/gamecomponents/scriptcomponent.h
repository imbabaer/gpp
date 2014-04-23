#pragma once

#include "gpp/gameObjectSystem.h"

namespace gpp
{
    class ScriptComponent : public Component
    {
    public:
        ScriptComponent();
        virtual ~ScriptComponent();

        virtual void initalize();
        virtual void update(float elapsedMS);
        virtual void destroy();

        void setInitializationFunction(const std::string& functionName);
        void setUpdateFunction(const std::string& functionName);
        void setDestroyFunction(const std::string& functionName);

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(setInitializationFunction)
            LUA_BIND_FUNCTION(setUpdateFunction)
            LUA_BIND_FUNCTION(setDestroyFunction)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        std::string m_luaInitializationFunction;
        std::string m_luaUpdateFunction;
        std::string m_luaDestroyFunction;
    };

    template<>
    struct ComponentMetaInfo<ScriptComponent>
    {
        static const char* name(){ return "ScriptComponent"; }
        static const int priority(){ return 42; }
        static ScriptComponent* create(){ return new ScriptComponent(); }
    };
}
