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

void gpp::CameraComponent::setActive()
{
    g_globalManager.getCameraManager()->setActiveCamera(m_pCamera);
}

