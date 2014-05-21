#pragma once

#include "gpp/gameObjectSystem.h"
#include "gep/interfaces/physics.h"

namespace gpp
{
    class PhysicsComponent : public Component, public ITransform
    {
        friend class CharacterComponent;
    public:
        PhysicsComponent();
        virtual ~PhysicsComponent();

        virtual void initalize() override;

        virtual void update(float elapsedMS) override;

        virtual void destroy() override;

        virtual void setPosition(const gep::vec3& pos) override;
        virtual void setRotation(const gep::Quaternion& rot) override;
        virtual void setScale(const gep::vec3& scale) override;

        virtual gep::vec3 getPosition() override;
        virtual gep::Quaternion getRotation() override;
        virtual gep::vec3 getScale() override;
        virtual gep::mat4 getTransformationMatrix() override;

        gep::IRigidBody* getRigidBody();

        gep::IRigidBody* createRigidBody(gep::RigidBodyCInfo& cinfo);

        inline gep::IWorld* getWorld() { return m_pWorld; }
        inline const gep::IWorld* getWorld() const { return m_pWorld; }
        inline void setWorld(gep::IWorld* world) { m_pWorld = world; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(createRigidBody)
            LUA_BIND_FUNCTION(getRigidBody)
        LUA_BIND_REFERENCE_TYPE_END 
    
    protected:
    private:
        void setRigidBody(gep::IRigidBody* rigidBody);
        gep::IRigidBody* m_pRigidBody;
        gep::IWorld* m_pWorld;
        bool m_ownsRigidBody;

    };
     
    template<>
    struct ComponentMetaInfo<PhysicsComponent>
    {
        static const char* name(){ return "PhysicsComponent"; }
        static const int priority(){ return -1; }
        static PhysicsComponent* create(){return new PhysicsComponent(); }
    };

}
