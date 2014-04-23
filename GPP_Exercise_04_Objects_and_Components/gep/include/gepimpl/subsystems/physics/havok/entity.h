#pragma once
#include "gep/interfaces/physics/entity.h"
#include "gep/interfaces/physics/contact.h"
#include "gep/container/DynamicArray.h"

#include "gepimpl/subsystems/physics/havok/conversion/vector.h"
#include "gepimpl/subsystems/physics/havok/conversion/quaternion.h"
#include "gepimpl/havok/util.h"

namespace gep
{
    class HavokContactListener : public hkpContactListener
    {
        friend class HavokEntity;

        IContactListener* m_pListener;
    public:

        HavokContactListener(IContactListener* listener) : m_pListener(listener)
        {
            GEP_ASSERT(m_pListener, "nullptr not allowed as contact listener!");
        }
        virtual ~HavokContactListener(){ m_pListener = nullptr; }

        virtual void contactPointCallback( const hkpContactPointEvent& evt ) override
        {
            auto source = static_cast<CollisionArgs::CallbackSource::Enum>(evt.m_source);
            auto* first = reinterpret_cast<IRigidBody*>(evt.m_bodies[0]->getUserData());
            auto* second = reinterpret_cast<IRigidBody*>(evt.m_bodies[1]->getUserData());
            GEP_ASSERT(m_pListener, "I have a nullptr as actual listener!");
            GEP_ASSERT(first, "Havok user data is null!");
            GEP_ASSERT(second, "Havok user data is null!");
            m_pListener->contactPointCallback(ContactPointArgs(source, first, second));
        }
        virtual void collisionAddedCallback( const hkpCollisionEvent& evt ) override
        {
            auto source = static_cast<CollisionArgs::CallbackSource::Enum>(evt.m_source);
            auto* first = reinterpret_cast<IRigidBody*>(evt.m_bodies[0]->getUserData());
            auto* second = reinterpret_cast<IRigidBody*>(evt.m_bodies[1]->getUserData());
            GEP_ASSERT(m_pListener, "I have a nullptr as actual listener!");
            GEP_ASSERT(first, "Havok user data is null!");
            GEP_ASSERT(second, "Havok user data is null!");
            m_pListener->collisionAddedCallback(ContactPointArgs(source, first, second));
        }
        virtual void collisionRemovedCallback( const hkpCollisionEvent& evt ) override
        {
            auto source = static_cast<CollisionArgs::CallbackSource::Enum>(evt.m_source);
            auto* first = reinterpret_cast<IRigidBody*>(evt.m_bodies[0]->getUserData());
            auto* second = reinterpret_cast<IRigidBody*>(evt.m_bodies[1]->getUserData());
            GEP_ASSERT(m_pListener, "I have a nullptr as actual listener!");
            GEP_ASSERT(first, "Havok user data is null!");
            GEP_ASSERT(second, "Havok user data is null!");
            m_pListener->collisionRemovedCallback(CollisionArgs(source, first, second));
        }
        virtual void contactPointAddedCallback( hkpContactPointAddedEvent& evt ) override
        {
            // deprecated?
        }
        virtual void contactPointRemovedCallback( hkpContactPointRemovedEvent& evt ) override
        {
            // deprecated?
        }
        virtual void contactProcessCallback( hkpContactProcessEvent& evt ) override
        {
            // deprecated?
        }
    };

    class HavokEntity : public virtual IPhysicsEntity
    {
        DynamicArray<HavokContactListener*> m_contactListeners;
    protected:
        hkpEntity* m_pEntity; ///< Must be set in a subclass!
    public:
        HavokEntity(hkpEntity* entity) : m_pEntity(entity)
        {
            if (m_pEntity)
                m_pEntity->addReference();
        }
        virtual ~HavokEntity(){ GEP_HK_REMOVE_REF_AND_NULL(m_pEntity); }

        virtual void addContactListener(IContactListener* listener)
        {
            GEP_ASSERT(listener);
            GEP_ASSERT(m_pEntity);
            auto* internalListener = new HavokContactListener(listener);
            m_pEntity->addContactListener(internalListener);
            m_contactListeners.append(internalListener);
        }

        virtual void removeContactListener(IContactListener* listener)
        {
            GEP_ASSERT(listener);
            GEP_ASSERT(m_pEntity);
            for (size_t i = 0; i < m_contactListeners.length(); i++)
            {
                auto* internalListener = m_contactListeners[i];
                if (internalListener->m_pListener == listener)
                {
                    m_contactListeners.removeAtIndex(i);
                    m_pEntity->removeContactListener(internalListener);
                    return;
                }
            }
        }
    };

    class HavokRigidBody : public HavokEntity, public IRigidBody
    {
        hkpTriggerVolume* m_pTriggerVolume;
        DynamicArray<IRigidBody::PositionChangedCallback> m_positionChangedCallbacks;
        gep::IShape* m_shape; ///< If != nullptr, we own this shape and must delete it.

    public:

        HavokRigidBody(hkpRigidBody* rigidBody = nullptr);
        HavokRigidBody(const RigidBodyCInfo& cinfo);

        virtual ~HavokRigidBody();

        inline       hkpRigidBody* getHkpRigidBody()       { return static_cast<hkpRigidBody*>(m_pEntity); }
        inline const hkpRigidBody* getHkpRigidBody() const { return static_cast<hkpRigidBody*>(m_pEntity); }

        virtual void initialize();
        virtual void destroy();

        virtual CallbackId registerSimulationCallback(PositionChangedCallback callback) override;
        virtual void deregisterSimulationCallback(CallbackId id) override;
        void triggerSimulationCallbacks() const;

        virtual uint32 getCollisionFilterInfo() const override { return getHkpRigidBody()->getCollisionFilterInfo(); }
        virtual void setCollisionFilterInfo(uint32 value) override { getHkpRigidBody()->setCollisionFilterInfo(value); }

        virtual float getMass() const override { return getHkpRigidBody()->getMass(); }
        virtual void setMass(float value) override { getHkpRigidBody()->setMass(value); }
        
        virtual Motion::Enum getMotionType() const override { return static_cast<Motion::Enum>(getHkpRigidBody()->getMotionType()); }
        virtual void setMotionType(Motion::Enum value) override { getHkpRigidBody()->setMotionType(static_cast<hkpMotion::MotionType>(value)); }
        
        virtual float getRestitution() const override { return getHkpRigidBody()->getRestitution(); }
        virtual void setRestitution(float value) override { getHkpRigidBody()->setRestitution(value); }

        virtual vec3 getPosition() const override { return conversion::hk::fromHkVector4(getHkpRigidBody()->getPosition()); }
        virtual void setPosition(const vec3& value) override { getHkpRigidBody()->setPosition(conversion::hk::toHkVector4(value)); }

        virtual Quaternion getRotation() const override { return conversion::hk::fromHkQuaternion(getHkpRigidBody()->getRotation()); }
        virtual void setRotation(const Quaternion& value) override { getHkpRigidBody()->setRotation(conversion::hk::toHkQuaternion(value)); }
        
        virtual float getFriction() const override { return getHkpRigidBody()->getFriction(); }
        virtual void setFriction(float value) override { return getHkpRigidBody()->setFriction(value); }
        
        virtual vec3 getLinearVelocity() const override { return conversion::hk::fromHkVector4(getHkpRigidBody()->getLinearVelocity()); }
        virtual void setLinearVelocity(const vec3& value) override { getHkpRigidBody()->setLinearVelocity(conversion::hk::toHkVector4(value)); }
        
        virtual vec3 getAngularVelocity() const override { return conversion::hk::fromHkVector4(getHkpRigidBody()->getAngularVelocity()); }
        virtual void setAngularVelocity(const vec3 & value) override { getHkpRigidBody()->setAngularVelocity(conversion::hk::toHkVector4(value)); }
        
        virtual float getLinearDamping() const override { return getHkpRigidBody()->getLinearDamping(); }
        virtual void setLinearDamping(float value) override { getHkpRigidBody()->setLinearDamping(value); }
        
        virtual float getAngularDamping() const override { return getHkpRigidBody()->getAngularDamping(); }
        virtual void setAngularDamping(float value) override { getHkpRigidBody()->setAngularDamping(value); }
        
        virtual float getGravityFactor() const override { return getHkpRigidBody()->getGravityFactor(); }
        virtual void setGravityFactor(float value) override { getHkpRigidBody()->setGravityFactor(value); }
        
        virtual float getRollingFrictionMultiplier() const override { return getHkpRigidBody()->getRollingFrictionMultiplier(); }
        virtual void setRollingFrictionMultiplier(float value) override { getHkpRigidBody()->setRollingFrictionMultiplier(value); }
        
        virtual float getMaxLinearVelocity() const override { return getHkpRigidBody()->getMaxLinearVelocity(); }
        virtual void setMaxLinearVelocity(float value) override { getHkpRigidBody()->setMaxLinearVelocity(value); }
        
        virtual float getMaxAngularVelocity() const override { return getHkpRigidBody()->getMaxAngularVelocity(); }
        virtual void setMaxAngularVelocity(float value) override { getHkpRigidBody()->setMaxAngularVelocity(value); }
        
        virtual float getTimeFactor() const override { return getHkpRigidBody()->getTimeFactor(); }
        virtual void setTimeFactor(float value) override { getHkpRigidBody()->setTimeFactor(value); }
        
        virtual uint16 getContactPointCallbackDelay() const override { return getHkpRigidBody()->getContactPointCallbackDelay(); }
        virtual void setContactPointCallbackDelay(uint16 value) override { getHkpRigidBody()->setContactPointCallbackDelay(value); }

        virtual void convertToTriggerVolume() override { if(!isTriggerVolume()) m_pTriggerVolume = new hkpTriggerVolume(static_cast<hkpRigidBody*>(m_pEntity)); }
        virtual bool isTriggerVolume() const override { return m_pTriggerVolume != nullptr; }

    private:

    };

    //TODO: Needs more wrapping!
    class HavokCollidable : public ICollidable
    {
        hkpCollidable* m_pHkCollidable;

        IPhysicsEntity* m_pEntity;
        IShape* m_pShape;
    public:

        HavokCollidable(hkpCollidable* collidable);

        virtual ~HavokCollidable(){}

        virtual void setOwner(IPhysicsEntity* owner) override { m_pEntity = owner; }
        virtual       IPhysicsEntity* getOwner() override       { return m_pEntity; }
        virtual const IPhysicsEntity* getOwner() const override { return m_pEntity; }

        virtual void setShape(IShape* shape) override { m_pShape = shape; }
        virtual       IShape* getShape()       override { return m_pShape; }
        virtual const IShape* getShape() const override { return m_pShape; }

              hkpCollidable* getHkCollidable()       { return m_pHkCollidable; }
        const hkpCollidable* getHkCollidable() const { return m_pHkCollidable; }

    };

}
