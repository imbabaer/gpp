#pragma once

#include "gep/singleton.h"
#include "gep/utils.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"
#include "gep/container/hashmap.h"
#include "gep/container/DynamicArray.h"
#include "gep/exception.h"
#include "gep/weakPtr.h"

#include "gep/interfaces/scripting.h"

namespace gpp
{
    class GameObject;

    class GameObjectManager: public gep::DoubleLockingSingleton<GameObjectManager>
    {
        friend class gep::DoubleLockingSingleton<GameObjectManager>;
    public:

        struct State
        {
            enum Enum
            {
                PreInitialization = 0,
                PostInitialization = 1
            };
        };

         GameObject* createGameObject(const std::string& guid);
         GameObject* getGameObject(const std::string& guid);

        virtual void initialize();
        virtual void destroy();
        virtual void update(float elapsedMs);

        State::Enum getState() { return m_state; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(createGameObject)
            LUA_BIND_FUNCTION(getGameObject)
        LUA_BIND_REFERENCE_TYPE_END 

    protected:
        GameObjectManager();
        virtual ~GameObjectManager();
    private:
       gep::Hashmap<std::string, GameObject*, gep::StringHashPolicy> m_gameObjects;
       State::Enum m_state;
    };

    class IComponent
    {
        friend class GameObject;
    public:

        struct State 
        {
            enum Enum
            {
                Initial,
                Active,
                Inactive
            };

            GEP_DISALLOW_CONSTRUCTION(State);
        };

        virtual ~IComponent() {}

        virtual void initalize() = 0;
        virtual void update(float elapsedMS) = 0;
        virtual void destroy() = 0;

        virtual void setState(State::Enum state) = 0;
        virtual State::Enum getState() const = 0;

        virtual       GameObject* getParentGameObject()       = 0;
        virtual const GameObject* getParentGameObject() const = 0;

    protected:
        virtual void setParentGameObject(GameObject* object) = 0;
    };

    /// \brief abstract base class of all components.
    class Component : public IComponent
    {
    public:
        Component() : m_pParentGameObject(nullptr), m_state(State::Initial) {}
        virtual ~Component() {}

        virtual       GameObject* getParentGameObject()       override { return m_pParentGameObject; }
        virtual const GameObject* getParentGameObject() const override { return m_pParentGameObject; }

        virtual void setState(State::Enum state) override { m_state = state; }
        virtual State::Enum getState() const override { return m_state; }

    protected:
        GameObject* m_pParentGameObject;
        State::Enum m_state;

        virtual void setParentGameObject(GameObject* object) override { m_pParentGameObject = object; }
    };

    class ITransform
    {
    public:
        virtual ~ITransform() {}

        virtual void setPosition(const gep::vec3& pos) = 0;
        virtual void setRotation(const gep::Quaternion& rot) = 0;
        virtual void setScale(const gep::vec3& scale) = 0;
        
        
        virtual gep::mat4 getTransformationMatrix() = 0;
        virtual gep::vec3 getPosition() = 0;
        virtual gep::Quaternion getRotation() = 0;
        virtual gep::vec3 getScale() = 0;
        virtual gep::vec3 getViewDirection() = 0;
        virtual gep::vec3 getUpDirection() = 0;
        virtual gep::vec3 getRightDirection() = 0;
    };

    class Transform : public ITransform
    {
    public:
        Transform():
            m_position(),
            m_scale(),
            m_rotation()
        {}
        virtual ~Transform() {}

        virtual void setPosition(const gep::vec3& pos) override { m_position = pos; }
        virtual void setRotation(const gep::Quaternion& rot) override { m_rotation = rot; }
        virtual void setScale(const gep::vec3& scale) override { m_scale = scale; }

        virtual gep::vec3 getPosition() override { return m_position; }
        virtual gep::Quaternion getRotation() override { return m_rotation; }
        virtual gep::vec3 getScale() override { return m_scale; }
        //TODO: extend for scale
        virtual gep::mat4 getTransformationMatrix() override { return gep::mat4::translationMatrix(m_position) * m_rotation.toMat4(); }
        virtual gep::vec3 getViewDirection() override {return m_rotation.toMat3() * gep::vec3(0,0,1);}
        virtual gep::vec3 getUpDirection() override {return m_rotation.toMat3() * gep::vec3(0,1,0);}
        virtual gep::vec3 getRightDirection() override {return m_rotation.toMat3() * gep::vec3(1,0,0);}

    private:
        gep::vec3 m_position;
        gep::vec3 m_scale;
        gep::Quaternion m_rotation;
    };

    class GameObject : public ITransform
    {
        friend class GameObjectManager;
        struct ComponentWrapper
        {
            int priority;
            IComponent* component;
        };

    public:
        GameObject();
        ~GameObject();

        void update(float elapsedMs);
        void initialize();
        void destroy();

        template<typename T>
        T* createComponent()
        {
            GEP_ASSERT(GameObjectManager::instance().getState() == GameObjectManager::State::PreInitialization, "You are not allowed to create game components after the initialization process.");
            T* instance = ComponentMetaInfo<T>::create();
            addComponent(instance);
            return instance;
        }

        template<typename T>
        T* getComponent()
        {
            return static_cast<T*>(m_components[ComponentMetaInfo<T>::name()]);
        }

        virtual void setPosition(const gep::vec3& pos) override;
        virtual void setRotation(const gep::Quaternion& rot) override;
        virtual void setScale(const gep::vec3& scale) override;

        virtual gep::vec3 getPosition() override;
        virtual gep::Quaternion getRotation() override;
        virtual gep::vec3 getScale() override;
        virtual gep::mat4 getTransformationMatrix() override;

        virtual gep::vec3 getViewDirection();
        virtual gep::vec3 getUpDirection();
        virtual gep::vec3 getRightDirection();

        void setComponentStates(IComponent::State::Enum value);

        inline const std::string& getName() const { return m_name; }

        inline       ITransform& getTransform()       { return *m_transform; }
        inline const ITransform& getTransform() const { return *m_transform; }
        inline void setTransform(ITransform& transform) { m_transform = &transform; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(createComponent<CameraComponent>, "createCameraComponent")
            LUA_BIND_FUNCTION_NAMED(createComponent<RenderComponent>, "createRenderComponent")
            LUA_BIND_FUNCTION_NAMED(createComponent<PhysicsComponent>, "createPhysicsComponent")
            LUA_BIND_FUNCTION_NAMED(createComponent<ScriptComponent>, "createScriptComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<CameraComponent>, "getCameraComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<RenderComponent>, "getRenderComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<PhysicsComponent>, "getPhysicsComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<ScriptComponent>, "getScriptComponent")
            LUA_BIND_FUNCTION(setPosition)
            LUA_BIND_FUNCTION(getPosition)
            LUA_BIND_FUNCTION(setRotation)
            LUA_BIND_FUNCTION(getRotation)
            LUA_BIND_FUNCTION(getViewDirection)
            LUA_BIND_FUNCTION(getUpDirection)
            LUA_BIND_FUNCTION(getRightDirection)
            LUA_BIND_FUNCTION(setComponentStates)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        std::string m_name;
        bool m_isActive;
        Transform m_defaultTransform;
        ITransform* m_transform;
        gep::Hashmap<const char*, IComponent*> m_components;
        gep::DynamicArray<ComponentWrapper> m_updateQueue;

        template<typename T>
        void addComponent(T* specializedComponent)
        {
            GEP_ASSERT(GameObjectManager::instance().getState() == GameObjectManager::State::PreInitialization, "You are not allowed to create game components after the initialization process.");
            //check weather T is really an ICompontent
            auto component = static_cast<IComponent*>(specializedComponent);
            ComponentWrapper wrapper;
            wrapper.priority = ComponentMetaInfo<T>::priority();
            wrapper.component = component;

            const char* const typeName = ComponentMetaInfo<T>::name();
            GEP_ASSERT(m_components[typeName] == nullptr, "A component of the same type has already been added to this gameObject", typeName, m_name);
            if (m_components[typeName] != nullptr)
            {
                throw gep::Exception(gep::format("The component %s has already been added to gameObject %s", typeName, m_name));
            }
            component->setParentGameObject(this);
            m_components[typeName] = component;

            if (wrapper.priority < 0)
            {
                return; //Component needs no update
            }
            //insert into update Queue
            size_t index = 0;

            for (auto entry : m_updateQueue)
            {
                if (entry.priority > wrapper.priority)
                {
                    break;
                }
                ++index;
            }
            m_updateQueue.insertAtIndex(index, wrapper);
        }
    };

    template<typename T>
    struct ComponentMetaInfo
    {
        static const char* name() { static_assert(false, "Please specialize this template in the specific component class!"); return nullptr;}
        static const int priority() { static_assert(false, "Please specialize this template in the specific component class!"); return nullptr;}
        static T* create() { static_assert(false, "Please specialize this template in the specific component class!"); return nullptr;}
    };

}

#define g_gameObjectManager (::gpp::GameObjectManager::instance())
