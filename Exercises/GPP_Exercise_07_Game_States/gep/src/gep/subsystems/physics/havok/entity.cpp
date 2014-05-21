#include "stdafx.h"

#include "gep/interfaces/physics/shape.h"

#include "gepimpl/subsystems/physics/havok/entity.h"
#include "gepimpl/subsystems/physics/havok/conversion/shape.h"

gep::HavokRigidBody::HavokRigidBody(hkpRigidBody* rigidBody) :
    m_pTriggerVolume(nullptr),
    m_positionChangedCallbacks(),
    m_shape(),
    m_entity(rigidBody)
{
    GEP_ASSERT(rigidBody && m_entity.getHkpEntity(), "Must not pass a nullptr!");
}

gep::HavokRigidBody::HavokRigidBody(const RigidBodyCInfo& cinfo) :
    m_pTriggerVolume(nullptr),
    m_positionChangedCallbacks(),
    m_shape(cinfo.shape),
    m_entity(nullptr)
{
    hkpRigidBodyCinfo hkcinfo;

    GEP_ASSERT(cinfo.shape, "Did not supply valid shape in RigidBodyCInfo!", cinfo.shape);

    // Construct the correct shape
    hkcinfo.m_shape = conversion::hk::to(cinfo.shape);

    // Set up mass of the rigid body.
    //TODO: Should this be done only if cinfo.motionType != Motion::Fixed?
    {
        const hkReal mass = cinfo.mass;

        hkMassProperties massProperties;
        hkpInertiaTensorComputer::computeShapeVolumeMassProperties(hkcinfo.m_shape, mass, massProperties);

        hkcinfo.setMassProperties(massProperties);
    }

    // set up the rest of the parameters
    hkcinfo.m_collisionFilterInfo = cinfo.collisionFilterInfo;
    hkcinfo.m_motionType = static_cast<hkpMotion::MotionType>(cinfo.motionType);
    conversion::hk::to(cinfo.position, hkcinfo.m_position);
    conversion::hk::to(cinfo.rotation, hkcinfo.m_rotation);
    hkcinfo.m_restitution = cinfo.restitution;
    hkcinfo.m_friction = cinfo.friction;
    conversion::hk::to(cinfo.linearVelocity, hkcinfo.m_linearVelocity);
    conversion::hk::to(cinfo.angularVelocity, hkcinfo.m_angularVelocity);
    hkcinfo.m_linearDamping = cinfo.linearDamping;
    hkcinfo.m_angularDamping = cinfo.angularDamping;
    hkcinfo.m_gravityFactor = cinfo.gravityFactor;
    hkcinfo.m_rollingFrictionMultiplier = cinfo.rollingFrictionMultiplier;
    hkcinfo.m_maxLinearVelocity = cinfo.maxLinearVelocity;
    hkcinfo.m_maxAngularVelocity = cinfo.maxAngularVelocity;
    hkcinfo.m_enableDeactivation = cinfo.enableDeactivation;
    hkcinfo.m_timeFactor = cinfo.timeFactor;
    hkcinfo.m_contactPointCallbackDelay = cinfo.contactPointCallbackDelay;
    hkcinfo.m_autoRemoveLevel = cinfo.autoRemoveLevel;
    hkcinfo.m_responseModifierFlags = cinfo.responseModifierFlags;
    hkcinfo.m_numShapeKeysInContactPointProperties = cinfo.numShapeKeysInContactPointProperties;

    m_entity.setHkpEntity(new hkpRigidBody(hkcinfo));
    if (hkcinfo.m_shape) hkcinfo.m_shape->removeReference();
}

gep::HavokRigidBody::~HavokRigidBody()
{
    m_entity.getHkpEntity()->setUserData(0);
    m_pTriggerVolume = nullptr;
}

void gep::HavokRigidBody::initialize()
{
    m_entity.getHkpEntity()->setUserData(reinterpret_cast<hkUlong>(this));
}

gep::CallbackId gep::HavokRigidBody::registerSimulationCallback(PositionChangedCallback callback)
{
    GEP_ASSERT(callback);
    for(size_t i=0; i < m_positionChangedCallbacks.length(); ++i)
    {
        if(!m_positionChangedCallbacks[i])
        {
            m_positionChangedCallbacks[i] = callback;
            return CallbackId(i);
        }
    }
    m_positionChangedCallbacks.append(callback);
    return gep::CallbackId(m_positionChangedCallbacks.length() - 1);
}

void gep::HavokRigidBody::deregisterSimulationCallback(CallbackId id)
{
    GEP_ASSERT(id.id < m_positionChangedCallbacks.length(), "callback id out of bounds");
    GEP_ASSERT(m_positionChangedCallbacks[id.id], "callback was already deregistered");
    m_positionChangedCallbacks[id.id] = nullptr;
}

void gep::HavokRigidBody::triggerSimulationCallbacks() const
{
    for (auto& callback : m_positionChangedCallbacks)
    {
        if (callback)
            callback(this);
    }
}

//////////////////////////////////////////////////////////////////////////

gep::HavokCollidable::HavokCollidable(hkpCollidable* collidable) :
    m_pHkCollidable(collidable),
    m_pShape(nullptr),
    m_pEntity(nullptr)
{
    GEP_ASSERT(collidable, "The input 'collidable' is not supposed to be null!");
    m_pEntity = static_cast<IPhysicsEntity*>(collidable->getOwner());

    m_pShape = conversion::hk::from( const_cast<hkpShape*>(collidable->getShape()) );
}
