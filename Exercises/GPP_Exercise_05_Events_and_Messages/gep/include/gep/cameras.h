#pragma once


#include "gep/interfaces/renderer.h"
#include "gep/gepmodule.h"
#include "gep/math3d/quaternion.h"
#include "gep/math3d/vec3.h"
#include "gep/interfaces/game.h"

namespace gep
{
    class GEP_API Camera : public ICamera
    {
    public:
		Camera();
        virtual ~Camera() {}

        virtual const mat4 getProjectionMatrix() const override;
        virtual const mat4 getViewMatrix() const override; 

        float getViewAngle() const { return m_viewAngleInDegrees; }
        void setViewAngle(float degrees) { m_viewAngleInDegrees = degrees; }

        float getAspectRatio() const { return m_aspectRatio; }
        void setAspectRatio(float value) { m_aspectRatio = value; }

        float getNear() const { return m_near; }
        void setNear(float value) { m_near = value; }

        float getFar() const { return m_far; }
        void setFar(float value) { m_far = value; }


    protected:
        float m_viewAngleInDegrees;
        float m_aspectRatio;
        float m_near;
        float m_far;
        Quaternion m_rotation;
        vec3 m_position;
    };
    
    class GEP_API FreeCamera : public Camera
    {


    public:
        FreeCamera();
        virtual ~FreeCamera(){}

        /// \brief makes the camera look around
        virtual void look(vec2 delta);

        /// \briefs moves the camera
        virtual void move(vec3 delta);

        /// \brief rotates the camera around the view axis
        virtual void tilt(float amount);

        virtual void setPosition(const gep::vec3& pos) {
            m_position = pos; 
        }
        virtual gep::vec3 getPosition() { 
            return m_position; 
        };   

        virtual void setRotation(const gep::Quaternion& rot) { m_rotation = rot; }
        virtual gep::Quaternion getRotation() { return m_rotation; };
        void lookAt(const gep::vec3& target);

        
        gep::vec3 const getUpVector();

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getPosition)
            LUA_BIND_FUNCTION(setPosition)
            LUA_BIND_FUNCTION(look)
            LUA_BIND_FUNCTION(tilt)
            LUA_BIND_FUNCTION(move)
            LUA_BIND_FUNCTION(lookAt)
            LUA_BIND_FUNCTION(getRotation)
            LUA_BIND_FUNCTION(setRotation)
		LUA_BIND_REFERENCE_TYPE_END

    

    };

    class GEP_API FreeCameraHorizon : public FreeCamera
    {
    protected:
        vec3 m_viewDir;
        vec2 m_rotation;
        float m_tilt;

    public:
        FreeCameraHorizon();
        virtual ~FreeCameraHorizon(){}

        virtual void look(vec2 delta) override;

        virtual void tilt(float amount) override;

        virtual void move(vec3 delta) override;

        const mat4 getViewMatrix() const override;

		virtual void setPosition(const gep::vec3& pos);

		virtual void setRotation(const gep::Quaternion& rot);

		virtual gep::vec3 getPosition();

		virtual gep::Quaternion getRotation();

    };

    class GEP_API CameraLookAtHorizon : public Camera
    {
    public:
        CameraLookAtHorizon();
        virtual void tilt(float amount);

        const mat4 getViewMatrix() const override;
        
        void lookAt(const gep::vec3& target);

        void setUpVector(const vec3& vector);

        virtual void setPosition(const gep::vec3& pos);

        virtual void setRotation(const gep::Quaternion& rot);

        virtual gep::vec3 getPosition();

        virtual gep::Quaternion getRotation();
        
        void look(const vec2& delta);

        void move(const vec3& delta);

    protected:
        vec3 m_targetPos;
        vec3 m_upVector;

        float m_tilt;
    };
       



    class GEP_API ThirdPersonCamera : public Camera
    {
    public:
        enum FollowMode
        {
            Direct,
            Smooth
        };

    private:
        const vec3 m_offset;
        vec3 m_position;
        mat4 m_matrixToFollow;
        mat4 m_fixedRotation;
        FollowMode m_followMode;

    public:
        ThirdPersonCamera(vec3 offset);
        virtual ~ThirdPersonCamera(){}

        void follow(const mat4& matrixToFollow);

        FollowMode getFollowMode() const;
        void setFollowMode(FollowMode followMode);

        virtual const mat4 getViewMatrix() const override;
    };
}

