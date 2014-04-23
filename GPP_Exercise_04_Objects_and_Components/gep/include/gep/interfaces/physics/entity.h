#pragma once
#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"
#include "gep/interfaces/updateFramework.h"

#include <functional>

#include "gep/interfaces/scripting.h"

namespace gep
{
    class IContactListener;
    class IShape;

    class IPhysicsEntity
    {
        friend class IPhysicsFactory;
    public:
        virtual ~IPhysicsEntity() {}

        virtual void initialize() = 0;
        virtual void destroy() = 0;

        virtual void addContactListener(IContactListener* listener) = 0;
        virtual void removeContactListener(IContactListener* listener) = 0;
    };

    /// \brief Purely static struct that serves as an enum.
    struct Motion
    {
        enum Enum
        {
            Invalid,
            Dynamic,
            SphereIntertia,
            BoxInertia,
            MotionKeyframed,
            Fixed,
            ThinBoxInertia,
            Character,
            MaxId
        };

        GEP_DISALLOW_CONSTRUCTION(Motion);
    };

    struct RigidBodyCInfo
    {
        uint32 collisionFilterInfo;
        float mass;

        IShape* shape;
        Motion::Enum motionType;
        float restitution;
        vec3 position;
        Quaternion rotation;
        float friction;
        vec3 linearVelocity;
        vec3 angularVelocity;
        float linearDamping;
        float angularDamping;
        float gravityFactor;
        float rollingFrictionMultiplier;
        float maxLinearVelocity;
        float maxAngularVelocity;
        bool enableDeactivation;
        // ??? solverDeactivation;
        float timeFactor;
        // ??? localFrame;
        // ??? collisionResponse;
        uint16 contactPointCallbackDelay;
        // ??? qualityType;
        int8 autoRemoveLevel;
        uint8 responseModifierFlags;
        int8 numShapeKeysInContactPointProperties;

        bool isTriggerVolume;
        bool ownsShape;

        struct SolverDeactivation
        {
            enum Enum
            {
                Invalid, ///<
                Off,     ///< No solver deactivation
                Low,     ///< Very conservative deactivation, typically no visible artifacts.
                Medium,  ///< Normal deactivation, no serious visible artifacts in most cases
                High,    ///< Fast deactivation, visible artifacts
                Max      ///< Very fast deactivation, visible artifacts
            };

            GEP_DISALLOW_CONSTRUCTION(SolverDeactivation);
        };

        inline RigidBodyCInfo() :
            collisionFilterInfo(0U),
            mass(0.0f),
            shape(nullptr),
            motionType(Motion::Fixed),
            restitution(0.4f),
            position(0.0f),
            rotation(),
            friction(0.5f),
            linearVelocity(0.0f),
            angularVelocity(0.0f),
            linearDamping(0.0f),
            angularDamping(0.0f),
            gravityFactor(1.0f),
            rollingFrictionMultiplier(0.0f),
            maxLinearVelocity(200.0f),
            maxAngularVelocity(200.0f),
            enableDeactivation(true),
            timeFactor(1.0f),
            contactPointCallbackDelay(0xffff),
            autoRemoveLevel(0),
            responseModifierFlags(0),
            numShapeKeysInContactPointProperties(0),
            isTriggerVolume(false),
            ownsShape(false)
        {
        }

            LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_VALUE_TYPE_MEMBERS
                LUA_BIND_MEMBER(collisionFilterInfo)
                LUA_BIND_MEMBER(mass)
                LUA_BIND_MEMBER(shape)
                LUA_BIND_MEMBER(motionType)
                LUA_BIND_MEMBER(restitution)
                LUA_BIND_MEMBER(position)
                LUA_BIND_MEMBER(rotation)
                LUA_BIND_MEMBER(friction)
                LUA_BIND_MEMBER(linearVelocity)
                LUA_BIND_MEMBER(angularVelocity)
                LUA_BIND_MEMBER(linearDamping)
                LUA_BIND_MEMBER(angularDamping)
                LUA_BIND_MEMBER(gravityFactor)
                LUA_BIND_MEMBER(rollingFrictionMultiplier)
                LUA_BIND_MEMBER(maxLinearVelocity)
                LUA_BIND_MEMBER(maxAngularVelocity)
                LUA_BIND_MEMBER(enableDeactivation)
                LUA_BIND_MEMBER(timeFactor)
                LUA_BIND_MEMBER(contactPointCallbackDelay)
                LUA_BIND_MEMBER(autoRemoveLevel)
                LUA_BIND_MEMBER(responseModifierFlags)
                LUA_BIND_MEMBER(numShapeKeysInContactPointProperties)
                LUA_BIND_MEMBER(isTriggerVolume)
            LUA_BIND_VALUE_TYPE_END
    };

    class IRigidBody : public virtual IPhysicsEntity
    {
    public:
        typedef std::function<void(const IRigidBody*)> PositionChangedCallback;

        virtual ~IRigidBody(){}


        virtual uint32 getCollisionFilterInfo() const = 0;
        virtual void setCollisionFilterInfo(uint32 value) = 0;

        virtual float getMass() const = 0;
        virtual void setMass(float value) = 0;

        virtual Motion::Enum getMotionType() const = 0;
        virtual void setMotionType(Motion::Enum value) = 0;

        virtual float getRestitution() const = 0;
        virtual void setRestitution(float value) = 0;

        virtual vec3 getPosition() const = 0;
        virtual void setPosition(const vec3& value) = 0;

        virtual Quaternion getRotation() const = 0;
        virtual void setRotation(const Quaternion& value) = 0;

        virtual float getFriction() const = 0;
        virtual void setFriction(float value) = 0;

        virtual vec3 getLinearVelocity() const = 0;
        virtual void setLinearVelocity(const vec3& value) = 0;

        virtual vec3 getAngularVelocity() const = 0;
        virtual void setAngularVelocity(const vec3 & value) = 0;

        virtual float getLinearDamping() const = 0;
        virtual void setLinearDamping(float value) = 0;

        virtual float getAngularDamping() const = 0;
        virtual void setAngularDamping(float value) = 0;

        virtual float getGravityFactor() const = 0;
        virtual void setGravityFactor(float value) = 0;

        virtual float getRollingFrictionMultiplier() const = 0;
        virtual void setRollingFrictionMultiplier(float value) = 0;

        virtual float getMaxLinearVelocity() const = 0;
        virtual void setMaxLinearVelocity(float value) = 0;

        virtual float getMaxAngularVelocity() const = 0;
        virtual void setMaxAngularVelocity(float value) = 0;

        virtual float getTimeFactor() const = 0;
        virtual void setTimeFactor(float value) = 0;

        virtual uint16 getContactPointCallbackDelay() const = 0;
        virtual void setContactPointCallbackDelay(uint16 value) = 0;

        /// \brief converts this rigid body to a trigger volume.
        virtual void convertToTriggerVolume() = 0;
        virtual bool isTriggerVolume() const = 0;

        virtual CallbackId registerSimulationCallback(PositionChangedCallback callback) = 0;
        virtual void deregisterSimulationCallback(CallbackId id) = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getLinearVelocity)
            LUA_BIND_FUNCTION(setLinearVelocity)
			LUA_BIND_FUNCTION(getAngularVelocity)
			LUA_BIND_FUNCTION(setAngularVelocity)
            LUA_BIND_FUNCTION(getRotation)
            LUA_BIND_FUNCTION(setRotation)
        LUA_BIND_REFERENCE_TYPE_END

    };

    //TODO: Needs more wrapping!
    class ICollidable
    {
    public:
        virtual ~ICollidable(){}

        virtual void setOwner(IPhysicsEntity* owner) = 0;
        virtual IPhysicsEntity* getOwner() = 0;
        virtual const IPhysicsEntity* getOwner() const = 0;

        virtual void setShape(IShape* shape) = 0;
        virtual IShape* getShape() = 0;
        virtual const IShape* getShape() const = 0;
    };
}
