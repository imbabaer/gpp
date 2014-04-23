#include "stdafx.h"
#include "gpp/gameComponents/scriptComponent.h"
#include "gep\globalManager.h"
#include "gep\interfaces\scripting.h"


gpp::ScriptComponent::ScriptComponent() :
    m_luaInitializationFunction(""),
    m_luaUpdateFunction(""),
    m_luaDestroyFunction("")
{

}

gpp::ScriptComponent::~ScriptComponent()
{

}

void gpp::ScriptComponent::initalize()
{
    if (!m_luaInitializationFunction.empty())
    {
        g_globalManager.getScriptingManager()->callFunction<void>(
            m_luaInitializationFunction.c_str(),   ///< Function name
            m_pParentGameObject->getName() ///< ID of the game object
            );
    }
}

void gpp::ScriptComponent::update(float elapsedMS)
{
    if (!m_luaUpdateFunction.empty())
    {
        g_globalManager.getScriptingManager()->callFunction<void>(
            m_luaUpdateFunction.c_str(),    ///< Function name
            m_pParentGameObject->getName(), ///< ID of the game object
            elapsedMS                       ///< Elapsed time in milliseconds
        );
    }
}

void gpp::ScriptComponent::destroy()
{
    if (!m_luaDestroyFunction.empty())
    {
        g_globalManager.getScriptingManager()->callFunction<void>(
            m_luaDestroyFunction.c_str(),   ///< Function name
            m_pParentGameObject->getName() ///< ID of the game object
            );
    }
}

void gpp::ScriptComponent::setInitializationFunction(const std::string& functionName)
{
    m_luaInitializationFunction = functionName;
}

void gpp::ScriptComponent::setUpdateFunction(const std::string& functionName)
{
    m_luaUpdateFunction = functionName;
}

void gpp::ScriptComponent::setDestroyFunction(const std::string& functionName)
{
    m_luaDestroyFunction = functionName;
}
