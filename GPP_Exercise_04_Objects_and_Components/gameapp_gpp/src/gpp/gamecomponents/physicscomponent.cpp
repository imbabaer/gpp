#include "stdafx.h"
#include "gpp/gameComponents/physicsComponent.h"

#include "gep/interfaces/physics.h"
#include "gep/interfaces/physics/factory.h"
#include "gep/globalManager.h"

gpp::PhysicsComponent::PhysicsComponent():
    Component(),
    m_pRigidBody(nullptr),
    m_pWorld(g_globalManager.getPhysicsSystem()->getWorld()),
    m_ownsRigidBody(true)
{
}

gpp::PhysicsComponent::~PhysicsComponent()
{
    if(m_ownsRigidBody)
    {
        DELETE_AND_NULL(m_pRigidBody);
    } 
    else
    {
        m_pRigidBody = nullptr;
    }

    m_pWorld = nullptr;
}

void gpp::PhysicsComponent::initalize()
{
}

void gpp::PhysicsComponent::update(float elapsedMS)
{
    
}

void gpp::PhysicsComponent::destroy()
{
    if(m_ownsRigidBody && m_pRigidBody)
    {
        m_pWorld->removeEntity(m_pRigidBody);
        m_pRigidBody->destroy();
    }
}

void gpp::PhysicsComponent::setPosition(const gep::vec3& pos)
{
    m_pRigidBody->setPosition(pos);
}

void gpp::PhysicsComponent::setRotation(const gep::Quaternion& rot)
{
    m_pRigidBody->setRotation(rot);
}

void gpp::PhysicsComponent::setScale(const gep::vec3& scale)
{
    GEP_ASSERT(false, "The engine currently doesn't support scaling of rigid bodies!");
}

gep::mat4 gpp::PhysicsComponent::getTransformationMatrix()
{
    //TODO: Extend for scale
    return gep::mat4::translationMatrix(m_pRigidBody->getPosition()) * m_pRigidBody->getRotation().toMat4();
}

gep::vec3 gpp::PhysicsComponent::getPosition()
{
    return m_pRigidBody->getPosition();
}

gep::Quaternion gpp::PhysicsComponent::getRotation()
{
    return m_pRigidBody->getRotation();
}

gep::vec3 gpp::PhysicsComponent::getScale()
{
   GEP_ASSERT(false, "The engine currently doesn't support scaling of rigid bodies!");
   return gep::vec3(1.0f, 1.0f, 1.0f);
}

gep::IRigidBody* gpp::PhysicsComponent::getRigidBody()
{
    return m_pRigidBody;
}

gep::IRigidBody* gpp::PhysicsComponent::createRigidBody(gep::RigidBodyCInfo& cinfo )
{
    GEP_ASSERT(m_pWorld != nullptr, "The Physics World for of the PhysicsComponent on has not been set.", m_pParentGameObject->getName().c_str());

    auto* physicsSystem = g_globalManager.getPhysicsSystem();
    auto* physicsFactory = physicsSystem->getPhysicsFactory();

    cinfo.ownsShape = true;
    m_pRigidBody = physicsFactory->createRigidBody(cinfo);
    m_pRigidBody->initialize();
    //Note: The world is taking ownership of the rigid body here.
    m_pWorld->addEntity(m_pRigidBody, gep::TakeOwnership::no);
    m_pParentGameObject->setTransform(*this);
    return m_pRigidBody;
}

// PRIVATE:
void gpp::PhysicsComponent::setRigidBody(gep::IRigidBody* rigidBody)
{
    m_pRigidBody = rigidBody;
    m_pParentGameObject->setTransform(*this);
    m_ownsRigidBody = false;
}
