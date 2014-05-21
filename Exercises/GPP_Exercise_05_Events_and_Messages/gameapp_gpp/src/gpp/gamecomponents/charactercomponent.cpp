#include "stdafx.h"
#include "gpp/gameComponents/characterComponent.h"

#include "gep/globalManager.h"
#include "gep/interfaces/physics.h"
#include "gep/interfaces/physics/characterController.h"
#include "gep/interfaces/physics/factory.h"
#include "gep/interfaces/inputHandler.h"
#include "gep/interfaces/logging.h"

#include "gep/interfaces/renderer.h"

#include "gpp/gameComponents/physicsComponent.h"


gpp::CharacterComponent::CharacterComponent():
    Component(),
    m_position(0.0f, 0.0f, 0.0f),
    m_rotation(),
    m_pCharacterRigidBody(nullptr)
{

}

gpp::CharacterComponent::~CharacterComponent()
{
}

void gpp::CharacterComponent::initalize()
{

}

void gpp::CharacterComponent::update( float elapsedMS )
{
    auto* pInputHandler = g_globalManager.getInputHandler();
    auto* pLogging = g_globalManager.getLogging();
    auto& debugRenderer = g_globalManager.getRenderer()->getDebugRenderer();

    if (m_pCharacterRigidBody)
    {
        gep::CharacterInput input;
        gep::CharacterOutput output;

        input.deltaTime = elapsedMS;
        input.velocity = m_pCharacterRigidBody->getRigidBody()->getLinearVelocity();
        input.position = m_pCharacterRigidBody->getRigidBody()->getPosition();

        if (pInputHandler->isPressed(gep::Key::Up))
        {
            pLogging->logMessage("up");
            input.inputUD = -1.0f;
        }
        if (pInputHandler->isPressed(gep::Key::Down))
        {
            pLogging->logMessage("down");
            input.inputUD = 1.0f;
        }
        if (pInputHandler->isPressed(gep::Key::Left))
        {
            pLogging->logMessage("left");
            input.inputLR = 1.0f;
        }
        if (pInputHandler->isPressed(gep::Key::Right))
        {
            pLogging->logMessage("right");
            input.inputLR = -1.0f;
        }

        input.forward.x = 1.0f;
        input.characterGravity.z = -9.81f;

        m_pCharacterRigidBody->checkSupport(elapsedMS, input.surfaceInfo);

        switch (input.surfaceInfo.supportedState)
        {
        case gep::SurfaceInfo::SupportedState::Supported:
            debugRenderer.printText(m_position + gep::vec3(0.0f, 0.0f, -10.0f), "Supported", gep::Color::yellow());
            break;
        case gep::SurfaceInfo::SupportedState::Unsupported:
            debugRenderer.printText(m_position + gep::vec3(0.0f, 0.0f, -10.0f), "Unsupported", gep::Color::yellow());
            break;
        case gep::SurfaceInfo::SupportedState::Sliding:
            debugRenderer.printText(m_position + gep::vec3(0.0f, 0.0f, -10.0f), "Sliding", gep::Color::yellow());
            break;
        default:
            break;
        }

        debugRenderer.printText(m_position + gep::vec3(0.0f, 0.0f, -5.0f),
            gep::CharacterState::toString(m_pCharacterRigidBody->getState()),
            gep::Color::yellow());

        m_pCharacterRigidBody->update(input, output);

        m_pCharacterRigidBody->setLinearVelocity(output.velocity, elapsedMS);
    }
}

void gpp::CharacterComponent::destroy()
{

}

void gpp::CharacterComponent::setPosition(const gep::vec3& pos )
{
    m_position = pos;
}

void gpp::CharacterComponent::setRotation(const gep::Quaternion& rot )
{
    m_rotation = rot;
}

void gpp::CharacterComponent::setScale(const gep::vec3& scale )
{
    GEP_ASSERT(false, "The engine currently doesn't support scaling of rigid bodies!");
}

gep::mat4 gpp::CharacterComponent::getTransformationMatrix()
{
    //TODO: Extend for scale
    return gep::mat4::translationMatrix(m_position) * m_rotation.toMat4();
}

gep::vec3 gpp::CharacterComponent::getPosition()
{
    return m_position;
}

gep::Quaternion gpp::CharacterComponent::getRotation()
{
    return m_rotation;
}

gep::vec3 gpp::CharacterComponent::getScale()
{
    GEP_ASSERT(false, "The engine currently doesn't support scaling of rigid bodies!");
    return gep::vec3(1,1,1);
}

void gpp::CharacterComponent::createCharacterRigidBody(gep::CharacterRigidBodyCInfo cinfo)
{
    GEP_ASSERT(m_pWorld != nullptr, "The Physics World for of the CharacterComponent on %s has not been set.", m_pParentGameObject->getName().c_str());

    GEP_ASSERT(m_pParentGameObject->getComponent<PhysicsComponent>() == nullptr, "The GameObject already has physical behavior. Currently no character can be added. TODO: FIX ME!");

    auto physicsComponent = m_pParentGameObject->createComponent<PhysicsComponent>();


    auto* physicsSystem = g_globalManager.getPhysicsSystem();
    auto* physicsFactory = physicsSystem->getPhysicsFactory();

    m_pCharacterRigidBody = physicsFactory->createCharacterRigidBody(cinfo);
    m_pCharacterRigidBody->initialize();
    //Note: The world is taking ownership of the rigid body here.
    m_pWorld->addCharacter(m_pCharacterRigidBody.get());
    
    physicsComponent->setRigidBody(m_pCharacterRigidBody->getRigidBody());
}

void gpp::CharacterComponent::move(const gep::vec3& in_delta)
{
    gep::vec3 delta(in_delta.x, in_delta.z, -in_delta.y);
    m_position += m_rotation.toMat3() * delta;
}



gep::vec3 gpp::CharacterComponent::getViewDirection()
{
    return m_rotation.toMat3() * gep::vec3(0,0,1);
}
gep::vec3 gpp::CharacterComponent::getUpDirection()
{
    return m_rotation.toMat3() * gep::vec3(0,1,0);
}
gep::vec3 gpp::CharacterComponent::getRightDirection()
{
    return m_rotation.toMat3() * gep::vec3(1,0,0);
}
