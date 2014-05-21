#pragma once

#include "gep/cameras.h"
#include "gpp/gameObjectSystem.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"

namespace gpp 
{
    // TODO inherit from ITransform
    class CameraComponent : public Component
    {
    public:
        CameraComponent();

        virtual ~CameraComponent();

        virtual void initalize();
        virtual void update(float elapsedMS);
        virtual void destroy();

        virtual void setPosition(const gep::vec3& pos);

        virtual void setRotation(const gep::Quaternion& rot);
        
        virtual gep::vec3 getPosition();

        virtual gep::Quaternion getRotation();

        void lookAt(const gep::vec3& target);

        void setActive();

        float getViewAngle() const {return m_pCamera->getViewAngle();}

        void setViewAngle(float angle){m_pCamera->setViewAngle(angle);}

        void tilt(float amount){m_pCamera->tilt(amount);}

        void move(const gep::vec3& delta){m_pCamera->move(delta);}

        void look(const gep::vec2& delta){m_pCamera->look(delta);}

        virtual void setState(State::Enum state) override;
    
        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(setPosition)
            LUA_BIND_FUNCTION(lookAt)
            LUA_BIND_FUNCTION(getPosition)
            LUA_BIND_FUNCTION(getRotation)
            LUA_BIND_FUNCTION(setRotation)
            LUA_BIND_FUNCTION(setState)
            LUA_BIND_FUNCTION(getViewAngle)
            LUA_BIND_FUNCTION(setViewAngle)
            LUA_BIND_FUNCTION(tilt)
            LUA_BIND_FUNCTION(move)
            LUA_BIND_FUNCTION(look)
        LUA_BIND_REFERENCE_TYPE_END 

    private:
        gep::CameraLookAtHorizon* m_pCamera;

    };

    template<>
    struct ComponentMetaInfo<CameraComponent>
    {
        static const char* name(){ return "CameraComponent"; }
        static const int priority(){ return 23; }
        static CameraComponent* create(){ return new CameraComponent(); }
    };
}
