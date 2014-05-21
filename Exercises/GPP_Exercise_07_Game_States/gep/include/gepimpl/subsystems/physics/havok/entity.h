#pragma once
#include "gep/interfaces/physics/entity.h"
#include "gep/interfaces/physics/contact.h"
#include "gep/container/DynamicArray.h"
#include "gep/ReferenceCounting.h"

#include "gepimpl/subsystems/physics/havok/contact.h"
#include "gepimpl/subsystems/physics/havok/conversion/vector.h"
#include "gepimpl/subsystems/physics/havok/conversion/quaternion.h"
#include "gepimpl/havok/util.h"

namespace gep
{
    
    class HavokEntity : public IPhysicsEntity
    {
        DynamicArray<HavokContactListener*> m_contactListeners;
    protected:
        hkRefPtr<hkpEntity> m_pEntity; ///< Must be set in a subclass, e.g. via the base ctor!
    public:
        HavokEntity(hkpEntity* entity) :
            m_pEntity(nullptr)
        {
            setHkpEntity(entity);
        }

        virtual ~HavokEntity()
        {
            for (auto contactListener : m_contactListeners)
            {
                m_pEntity->removeContactListener(contactListener);
                deleteAndNull(contactListener);
            }
            m_contactListeners.clear();
        }

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
                auto internalListener = m_contactListeners[i];
                if (internalListener->getActualListener() == listener)
                {
                    m_contactListeners.removeAtIndex(i);
                    m_pEntity->removeContactListener(internalListener);
                    delete internalListener;
                    return;
                }
            }
        }

        inline       hkpEntity* getHkpEntity()       { return m_pEntity; }
        inline const hkpEntity* getHkpEntity() const { return m_pEntity; }
        inline void setHkpEntity(hkpEntity* entity) { m_pEntity = entity; }

        virtual void initialize() override {}

        virtual void activate() override { m_pEntity->activate(); }
        virtual void requestDeactivation() override { m_pEntity->requestDeactivation(); }
        virtual bool isActive() const override { return m_pEntity->isActive(); }

    };

    class HavokRigidBody : public IRigidBody
    {
        hkpTriggerVolume* m_pTriggerVolume;
        DynamicArray<IRigidBody::PositionChangedCallback> m_positionChangedCallbacks;
        SmartPtr<IShape> m_shape; ///< If != nullptr, we own this shape and must delete it.
        HavokEntity m_entity;
    public:

        HavokRigidBody(hkpRigidBody* rigidBody = nullptr);
        HavokRigidBody(const RigidBodyCInfo& cinfo);

        virtual ~HavokRigidBody();

        inline       hkpRigidBody* getHkpRigidBody()       { return static_cast<      hkpRigidBody*>(m_entity.getHkpEntity()); }
        inline const hkpRigidBody* getHkpRigidBody() const { return static_cast<const hkpRigidBody*>(m_entity.getHkpEntity()); }

        virtual void initialize() override;

        virtual CallbackId registerSimulationCallback(PositionChangedCallback callback) override;
        virtual void deregisterSimulationCallback(CallbackId id) override;
        void triggerSimulationCallbacks() const;

        virtual uint32 getCollisionFilterInfo() const override { return getHkpRigidBody()->getCollisionFilterInfo(); }
        virtual void setCollisionFilterInfo(uint32 value) override { getHkpRigidBody()->setCollisionFilterInfo(value); }

        virtual float getMass() const override { return getHkpRigidBody()->getMass(); }
        virtual void setMass(float value) override { getHkpRigidBody()->setMass(value); }
        
        virtual MotionType::Enum getMotionType() const override { return static_cast<MotionType::Enum>(getHkpRigidBody()->getMotionType()); }
        virtual void setMotionType(MotionType::Enum value) override { getHkpRigidBody()->setMotionType(static_cast<hkpMotion::MotionType>(value)); }
        
        virtual float getRestitution() const override { return getHkpRigidBody()->getRestitution(); }
        virtual void setRestitution(float value) override { getHkpRigidBody()->setRestitution(value); }

        virtual vec3 getPosition() const override { return conversion::hk::from(getHkpRigidBody()->getPosition()); }
        virtual void setPosition(const vec3& value) override { getHkpRigidBody()->setPosition(conversion::hk::to(value)); }

        virtual Quaternion getRotation() const override { return conversion::hk::from(getHkpRigidBody()->getRotation()); }
        virtual void setRotation(const Quaternion& value) override { getHkpRigidBody()->setRotation(conversion::hk::to(value)); }
        
        virtual float getFriction() const override { return getHkpRigidBody()->getFriction(); }
        virtual void setFriction(float value) override { return getHkpRigidBody()->setFriction(value); }
        
        virtual vec3 getLinearVelocity() const override { return conversion::hk::from(getHkpRigidBody()->getLinearVelocity()); }
        virtual void setLinearVelocity(const vec3& value) override { getHkpRigidBody()->setLinearVelocity(conversion::hk::to(value)); }
        
        virtual vec3 getAngularVelocity() const override { return conversion::hk::from(getHkpRigidBody()->getAngularVelocity()); }
        virtual void setAngularVelocity(const vec3 & value) override { getHkpRigidBody()->setAngularVelocity(conversion::hk::to(value)); }
        
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

        virtual void convertToTriggerVolume() override { if(!isTriggerVolume()) m_pTriggerVolume = new hkpTriggerVolume(getHkpRigidBody()); }
        virtual bool isTriggerVolume() const override { return m_pTriggerVolume != nullptr; }

        virtual void applyForce(float deltaSeconds, const vec3& force) override { getHkpRigidBody()->applyForce(deltaSeconds, conversion::hk::to(force)); }
        virtual void applyForceAt(float deltaSeconds, const vec3& force, const vec3& point) override { getHkpRigidBody()->applyForce(deltaSeconds, conversion::hk::to(force), conversion::hk::to(point)); }
        virtual void applyTorque(float deltaSeconds, const vec3& torque) override { getHkpRigidBody()->applyTorque(deltaSeconds, conversion::hk::to(torque)); }
        
        virtual void applyLinearImpulse(const vec3& impulse) override { getHkpRigidBody()->applyLinearImpulse(conversion::hk::to(impulse)); }
        virtual void applyAngularImpulse(const vec3& impulse) override { getHkpRigidBody()->applyAngularImpulse(conversion::hk::to(impulse)); }
        virtual void applyPointImpulse(const vec3& impulse, const vec3& point) override { getHkpRigidBody()->applyPointImpulse(conversion::hk::to(impulse), conversion::hk::to(point)); }

        // Forwards to the inner entity
        virtual void addContactListener(IContactListener* listener) override
        {
            m_entity.addContactListener(listener);
        }
        virtual void removeContactListener(IContactListener* listener) override
        {
            m_entity.removeContactListener(listener);
        }
        virtual void activate() override { m_entity.activate(); }
        virtual void requestDeactivation() override { m_entity.requestDeactivation(); }
        virtual bool isActive() const override { return m_entity.isActive(); }
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
