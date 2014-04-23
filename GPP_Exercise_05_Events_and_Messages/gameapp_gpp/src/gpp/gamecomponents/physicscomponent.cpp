#include "stdafx.h"
#include "gpp/gameComponents/physicsComponent.h"

#include "gep/interfaces/physics.h"
#include "gep/globalManager.h"
#include "gpp/ExperimentalContactListener.h"

gpp::PhysicsComponent::PhysicsComponent():
    Component(),
    m_pRigidBody(nullptr),
    m_pWorld(g_globalManager.getPhysicsSystem()->getWorld()),
    m_event_contactPoint()
{
}

gpp::PhysicsComponent::~PhysicsComponent()
{
    m_pRigidBody = nullptr;
    m_pWorld = nullptr;
}

void gpp::PhysicsComponent::initalize()
{

    if (m_state != State::Initial) { return; } // User already the state.
    setState(State::Active);
}

void gpp::PhysicsComponent::update(float elapsedMS)
{
}

void gpp::PhysicsComponent::destroy()
{
    setState(State::Inactive);
}

void gpp::PhysicsComponent::setPosition(const gep::vec3& pos)
{
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
    m_pRigidBody->setPosition(pos);
}

void gpp::PhysicsComponent::setRotation(const gep::Quaternion& rot)
{
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
    m_pRigidBody->setRotation(rot);
}

void gpp::PhysicsComponent::setScale(const gep::vec3& scale)
{
    GEP_ASSERT(false, "The engine currently doesn't support scaling of rigid bodies!");
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
}

gep::mat4 gpp::PhysicsComponent::getTransformationMatrix()
{
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
    //TODO: Extend for scale
    return gep::mat4::translationMatrix(m_pRigidBody->getPosition()) * m_pRigidBody->getRotation().toMat4();
}

gep::vec3 gpp::PhysicsComponent::getPosition()
{
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
    return m_pRigidBody->getPosition();
}

gep::Quaternion gpp::PhysicsComponent::getRotation()
{
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
    return m_pRigidBody->getRotation();
}

gep::vec3 gpp::PhysicsComponent::getScale()
{
   GEP_ASSERT(false, "The engine currently doesn't (and probably never will) support scaling of rigid bodies!");
   return gep::vec3(1.0f, 1.0f, 1.0f);
}

gep::IRigidBody* gpp::PhysicsComponent::getRigidBody()
{
    return m_pRigidBody.get();
}

gep::IRigidBody* gpp::PhysicsComponent::createRigidBody(gep::RigidBodyCInfo& cinfo)
{
    GEP_ASSERT(m_pWorld != nullptr, "The Physics World for of the PhysicsComponent on has not been set.", m_pParentGameObject->getName().c_str());
    GEP_ASSERT(!bool(m_pRigidBody), "The rigid body for of the PhysicsComponent has already been set.", m_pParentGameObject->getName().c_str());
    
    m_pParentGameObject->setTransform(*this); //TODO: Set parent gameObject in Component Constructor!

    auto* physicsSystem = g_globalManager.getPhysicsSystem();
    auto* physicsFactory = physicsSystem->getPhysicsFactory();

    m_pRigidBody = physicsFactory->createRigidBody(cinfo);
    m_pRigidBody->initialize();
    return m_pRigidBody.get();
}

// PRIVATE:
void gpp::PhysicsComponent::setRigidBody(gep::IRigidBody* rigidBody)
{
    m_pRigidBody = rigidBody;
    m_pParentGameObject->setTransform(*this);
}

void gpp::PhysicsComponent::contactPointCallback(const gep::ContactPointArgs& evt)
{
    
    m_event_contactPoint.trigger(&const_cast<gep::ContactPointArgs&>(evt));
}

void gpp::PhysicsComponent::collisionAddedCallback(const gep::CollisionArgs& evt)
{
}

void gpp::PhysicsComponent::collisionRemovedCallback(const gep::CollisionArgs& evt)
{
}



gep::vec3 gpp::PhysicsComponent::getViewDirection()
{
    return m_pRigidBody->getRotation().toMat3() * gep::vec3(0,0,1);
}
gep::vec3 gpp::PhysicsComponent::getUpDirection()
{
    return m_pRigidBody->getRotation().toMat3() * gep::vec3(0,1,0);
}
gep::vec3 gpp::PhysicsComponent::getRightDirection()
{
    return m_pRigidBody->getRotation().toMat3() * gep::vec3(1,0,0);
}

void gpp::PhysicsComponent::setState(State::Enum newState)
{
    GEP_ASSERT(newState != State::Initial);

    auto oldState = m_state;

    if (oldState == newState) { return; } // Nothing to do here.

    switch (newState)
    {
    case State::Active:
        if (oldState != State::Active)
        {
            activate();
        }
        break;
    case State::Inactive:
        if (oldState == State::Active)
        {
            deactivate();
        }
        break;
    default:
        GEP_ASSERT(false, "Invalid requested state.", oldState, newState);
        break;
    }

    m_state = newState;
}

void gpp::PhysicsComponent::activate()
{
    m_state = State::Active;
    m_pWorld->addEntity(m_pRigidBody.get());
    m_pRigidBody->addContactListener(this);
}

void gpp::PhysicsComponent::deactivate()
{
    m_state = State::Inactive;
    m_pRigidBody->removeContactListener(this);
    m_pWorld->removeEntity(m_pRigidBody.get());
}
