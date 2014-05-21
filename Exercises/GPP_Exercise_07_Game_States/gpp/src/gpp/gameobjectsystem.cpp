#include "stdafx.h"
#include "gpp/gameObjectSystem.h"

//GameObjectManager

//singleton static members
gpp::GameObjectManager* volatile gep::DoubleLockingSingleton<gpp::GameObjectManager>::s_instance = nullptr;
gep::Mutex gep::DoubleLockingSingleton<gpp::GameObjectManager>::s_creationMutex;

gpp::GameObjectManager::GameObjectManager():
    m_gameObjects(),
    m_state(State::PreInitialization)
{

}

gpp::GameObjectManager::~GameObjectManager()
{

}

gpp::GameObject* gpp::GameObjectManager::createGameObject(const std::string& guid)
{
    GEP_ASSERT(m_state == State::PreInitialization, "You are not allowed to create game objects after the initialization process.");

    GEP_ASSERT(m_gameObjects[guid] == nullptr, "GameObject %s already exists!", guid.c_str());
    auto gameObject = new GameObject();
    gameObject->m_name = guid;
    m_gameObjects[guid] = gameObject;
    return gameObject;
}

gpp::GameObject* gpp::GameObjectManager::getGameObject(const std::string& guid)
{
    return m_gameObjects[guid];
}

void gpp::GameObjectManager::initialize()
{
    for(auto pGameObject : m_gameObjects.values())
    {
        pGameObject->initialize();
    }
    m_state = State::PostInitialization;
}

void gpp::GameObjectManager::destroy()
{
    for(auto& gameObject : m_gameObjects.values())
    {
        gameObject->destroy();
        DELETE_AND_NULL(gameObject);
    }
    m_gameObjects.clear();
}

void gpp::GameObjectManager::update(float elapsedMs)
{
    for(auto gameObject : m_gameObjects.values())
    {
        gameObject->update(elapsedMs);
    }
}

gpp::GameObject::GameObject() :
    m_name(),
    m_isActive(true),
    m_defaultTransform(),
    m_transform(&m_defaultTransform),
    m_components(),
    m_updateQueue()
   
{
    
}

gpp::GameObject::~GameObject()
{

}

void gpp::GameObject::setPosition(const gep::vec3& pos)
{
    m_transform->setPosition(pos);
}

void gpp::GameObject::setRotation(const gep::Quaternion& rot)
{
    m_transform->setRotation(rot);
}

void gpp::GameObject::setScale(const gep::vec3& scale)
{
    m_transform->setScale(scale);
}

gep::vec3 gpp::GameObject::getPosition()
{
    return m_transform->getPosition();
}

gep::Quaternion gpp::GameObject::getRotation()
{
    return m_transform->getRotation();
}

gep::vec3 gpp::GameObject::getScale()
{
    return m_transform->getScale();
}

void gpp::GameObject::update(float elapsedMs)
{
    for(auto component :  m_updateQueue)
    {
        component.component->update(elapsedMs);
    }
}

void gpp::GameObject::initialize()
{
    for(auto component : m_components.values())
    {
        component->initalize();
        GEP_ASSERT(component->getState() != IComponent::State::Initial,
            "A game component must set its state within its initialize function!");
    }
}

void gpp::GameObject::destroy()
{
    // NOTE: Destroying in same order, not in reverse order, as initialization.
    for(auto& component : m_components.values())
    {
        component->destroy();
        DELETE_AND_NULL(component);
    }

    m_components.clear();
    m_updateQueue.resize(0);
}

gep::mat4 gpp::GameObject::getTransformationMatrix()
{
    return m_transform->getTransformationMatrix();
}

gep::vec3 gpp::GameObject::getViewDirection()
{
    return m_transform->getViewDirection();
}

gep::vec3 gpp::GameObject::getUpDirection()
{
    return m_transform->getUpDirection();
}

gep::vec3 gpp::GameObject::getRightDirection()
{
    return m_transform->getRightDirection();
}

void gpp::GameObject::setComponentStates(IComponent::State::Enum value)
{
    for (auto component : m_components.values())
    {
        component->setState(value);
    }
}
