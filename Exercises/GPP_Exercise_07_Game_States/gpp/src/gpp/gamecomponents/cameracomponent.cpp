#include "stdafx.h"

#include "gpp/gameComponents/cameraComponent.h"
#include "gpp/gameObjectSystem.h"
#include "gep/cameras.h"
#include "gep/globalManager.h"
#include "gep/interfaces/cameraManager.h"




gpp::CameraComponent::CameraComponent()
{
    m_pCamera = new gep::CameraLookAtHorizon();

}

gpp::CameraComponent::~CameraComponent()
{

}

void gpp::CameraComponent::initalize()
{
    setState(State::Active); // In case we were in the initial state.
}

void gpp::CameraComponent::update(float elapsedMS)
{

}

void gpp::CameraComponent::destroy()
{
    DELETE_AND_NULL(m_pCamera)
}

void gpp::CameraComponent::setPosition(const gep::vec3& pos)
{
    m_pCamera->setPosition(pos);
}

void gpp::CameraComponent::setRotation(const gep::Quaternion& rot)
{
    m_pCamera->setRotation(rot);
}

gep::vec3 gpp::CameraComponent::getPosition()
{
    return m_pCamera->getPosition();
}

gep::Quaternion gpp::CameraComponent::getRotation()
{
    return m_pCamera->getRotation();
}

void gpp::CameraComponent::lookAt(const gep::vec3& target)
{
    m_pCamera->lookAt(target);
}

void gpp::CameraComponent::setState(State::Enum state)
{
    GEP_ASSERT(state != State::Initial, "Cannot set the initial state!");
    
    m_state = state;

    switch (state)
    {
    case State::Active:
        g_globalManager.getCameraManager()->setActiveCamera(m_pCamera);
        break;
    case State::Inactive:
        GEP_ASSERT(false, "Cannot directly set a camera to be Inactive."
            "If you set any other camera to Active, all other cameras will "
            "automatically be set to be Inactive.");
        break;
    default:
        GEP_ASSERT(false, "Invalid state input!", state);
        break;
    }
}


