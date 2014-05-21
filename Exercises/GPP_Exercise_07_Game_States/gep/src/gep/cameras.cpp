#include "stdafx.h"
#include "gep/cameras.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/vec4.h"


gep::Camera::Camera() :
    m_aspectRatio(9.0f / 16.0f),
    m_viewAngleInDegrees(60),
    m_near(0.1f),
    m_far(10000.0f)
{

}

const gep::mat4 gep::Camera::getProjectionMatrix() const
{
    return mat4::projectionMatrix(m_viewAngleInDegrees, m_aspectRatio, m_near, m_far);
    //return mat4::ortho(-30.0f, 30.0f, -15.0f, 15.0f, m_near, m_far);
}


const gep::mat4 gep::Camera::getViewMatrix() const
{
    return (mat4::translationMatrix(m_position) * m_rotation.toMat4()).inverse();
}

//////////////////////////////////////////////////////////////////////////

gep::FreeCamera::FreeCamera()
{
    look(vec2(90.f, 0.f));
    tilt(-91.f);
}

gep::vec3 const gep::FreeCamera::getUpVector()
{
    const vec3 yZero = vec3(0, 0, 1);
    return getRotation().toMat3() * yZero;
}

void gep::FreeCamera::lookAt(const gep::vec3& target)
{
    mat4 lookAtMat = mat4::lookAtMatrix(getPosition(), target, getUpVector());
    m_rotation = Quaternion::fromMat4(lookAtMat);
}

void gep::FreeCamera::look(vec2 delta)
{
    auto qy = Quaternion(vec3(1, 0, 0), delta.y);
    auto qx = Quaternion(vec3(0, 1, 0), delta.x);
    m_rotation = qx * qy * m_rotation;
}

void gep::FreeCamera::move(vec3 delta)
{
    m_position += m_rotation.toMat3() * delta;
}

void gep::FreeCamera::tilt(float amount)
{
    m_rotation = Quaternion(vec3(0,0,1), amount) * m_rotation;
}

gep::FreeCameraHorizon::FreeCameraHorizon() :
    m_viewDir(1,0,0),
    m_rotation(0,0),
    m_tilt(0)
{
}

//////////////////////////////////////////////////////////////////////////

void gep::FreeCameraHorizon::look(vec2 delta)
{
    m_rotation.x += delta.x;
    m_rotation.y += delta.y;

    const float RotYMax = 75.f;
    m_rotation.y = (m_rotation.y < -RotYMax ? -RotYMax : (m_rotation.y > RotYMax ? RotYMax : m_rotation.y));

    m_viewDir = vec3(1,0,0);

    // left / right
    vec3 up(0,0,1);
    Quaternion qLeftRight(up, m_rotation.x);
    m_viewDir = qLeftRight.toMat3() * m_viewDir;

    // up / down
    vec3 left = m_viewDir.cross(up);
    Quaternion qUpDown(left, m_rotation.y);
    m_viewDir = qUpDown.toMat3() * m_viewDir;
}

void gep::FreeCameraHorizon::tilt(float amount)
{
    m_tilt += amount;
}

void gep::FreeCameraHorizon::move(vec3 delta)
{
    vec3 m_left = m_viewDir.cross(vec3(0,0,1));
    float angle = abs(m_viewDir.dot(vec3(0,0,1)));
    if(epsilonCompare(angle, GetPi<float>::value()/2.0f))
        m_left = vec3(0,1,0);
    m_position += m_viewDir * -delta.z + m_left * delta.x;
}

const gep::mat4 gep::FreeCameraHorizon::getViewMatrix() const
{
    vec3 up = Quaternion(m_viewDir, -m_tilt).toMat3() * vec3(0,0,1);
    return mat4::lookAtMatrix(m_position, m_position + m_viewDir, up);
}

//////////////////////////////////////////////////////////////////////////

gep::ThirdPersonCamera::ThirdPersonCamera(vec3 offset) :
    m_offset(offset),
    m_position(offset),
    m_followMode(Direct)
{
    m_fixedRotation = Quaternion(vec3(1,0,0), -80.0f).toMat4();
}

gep::ThirdPersonCamera::FollowMode gep::ThirdPersonCamera::getFollowMode() const
{
    return m_followMode;
}

void gep::ThirdPersonCamera::setFollowMode(FollowMode followMode)
{
    m_followMode = followMode;
}

void gep::ThirdPersonCamera::follow(const mat4& matrixToFollow)
{
    m_matrixToFollow = matrixToFollow;
    vec3 diff = (m_matrixToFollow.translationPart() - m_position) + (m_matrixToFollow.rotationPart() * m_offset);
    const float k = 0.05f; // spring constant
    m_position += diff * k;
}

const gep::mat4 gep::ThirdPersonCamera::getViewMatrix() const
{
    if (m_followMode == Direct)
        return (m_matrixToFollow * mat4::translationMatrix(m_offset) * m_fixedRotation).inverse();
    return mat4::lookAtMatrix(m_position, m_matrixToFollow.translationPart(), vec3(0,0,1));
}


//////////////////////////////////////////////////////////////////////////

void gep::FreeCameraHorizon::setPosition(const gep::vec3& pos)
{
    m_position = pos;
}

void gep::FreeCameraHorizon::setRotation(const gep::Quaternion& rot)
{

}

gep::vec3 gep::FreeCameraHorizon::getPosition()
{
    return m_position;
}

gep::Quaternion gep::FreeCameraHorizon::getRotation()
{
    return Quaternion(); // TODO: fix
}

//////////////////////////////////////////////////////////////////////////

gep::CameraLookAtHorizon::CameraLookAtHorizon() :
    m_upVector(0,0,1),
    m_viewDir(),
    m_tilt(0)
{
}

void gep::CameraLookAtHorizon::tilt(float amount)
{
    m_tilt += amount;
}

const  gep::mat4  gep::CameraLookAtHorizon::getViewMatrix() const
{
    vec3 up = Quaternion(m_viewDir, -m_tilt).toMat3() * m_upVector;

    return mat4::lookAtMatrix(m_position, m_position + m_viewDir, up);
}

void  gep::CameraLookAtHorizon::setUpVector(const vec3& vector)
{
    m_upVector = vector.normalized();
}

void gep::CameraLookAtHorizon::lookAt(const gep::vec3& target)
{
     m_viewDir = (target - m_position).normalized();

}


void  gep::CameraLookAtHorizon::setPosition(const gep::vec3& pos)
{
    m_position = pos;
}

void  gep::CameraLookAtHorizon::setRotation(const gep::Quaternion& rot)
{
   m_rotation = rot;
}

gep::vec3  gep::CameraLookAtHorizon::getPosition()
{
    return m_position;
}

gep::Quaternion  gep::CameraLookAtHorizon::getRotation()
{
    return m_rotation;
}

void gep::CameraLookAtHorizon::look(const vec2& delta)
{
    vec3 right = m_viewDir.cross(m_upVector);
    auto qy = Quaternion(right, delta.y);
    auto qx = Quaternion(m_upVector, delta.x);
    m_viewDir = (qy * qx).toMat3() * m_viewDir;
}

void gep::CameraLookAtHorizon::move(const vec3& delta)
{
    m_position += m_rotation.toMat3() * delta;
}

gep::vec3 gep::CameraLookAtHorizon::getUpVectorFromRotation() {return m_rotation.toMat3() * gep::vec3(0,1,0);}
gep::vec3 gep::CameraLookAtHorizon::getRightVectorFromRotation()  {return m_rotation.toMat3() * gep::vec3(1,0,0);}
