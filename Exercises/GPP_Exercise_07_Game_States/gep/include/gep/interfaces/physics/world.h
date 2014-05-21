#pragma once

#include "gep/interfaces/physics/rayCast.h"

#include "gep/interfaces/events.h"
#include "gep/interfaces/scripting.h"

namespace gep
{
    class IPhysicsEntity;
    class ICharacterRigidBody;
    class ContactPointArgs;

    /// \brief Construction info for a physics world instance.
    struct WorldCInfo
    {
        vec3 gravity;
        float worldSize;

        inline WorldCInfo() :
            gravity(0.0f),
            worldSize(100.0f)
        {
        }

        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(gravity)
            LUA_BIND_MEMBER(worldSize)
        LUA_BIND_VALUE_TYPE_END
    };

    class IWorld : public ReferenceCounted
    {
    public:
        virtual ~IWorld(){}

        /// \brief Adds a physics entity to this world.
        /// \param own If set to \c TakeOwnership::yes, this entity will be destroying once the world is destroyed.
        virtual void addEntity(IPhysicsEntity* entity) = 0;

        /// \brief Removes an entity from this world and transfers the ownership of it to the caller.
        virtual void removeEntity(IPhysicsEntity* entity) = 0;

        /// \brief Adds a character to the world.
        ///
        /// Will automatically add the underlying physics entity to this world as well.
        virtual void addCharacter(ICharacterRigidBody* character) = 0;

        /// \brief Removes a character from this world.
        ///
        /// Will automatically remove the underlying physics entity from this world as well.
        /// The ownership of the character rigid body is transfered to the caller.
        virtual void removeCharacter(ICharacterRigidBody* character) = 0;

        /// \brief Register a contact listener for all collision events
        virtual Event<ContactPointArgs*>* getContactPointEvent() = 0;

        /// \brief Casts a ray defined in \a input into this world. The output is represented by the \a output argument.
        virtual void castRay(const RayCastInput& input, RayCastOutput& output) const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            //LUA_BIND_FUNCTION(castRay)
            LUA_BIND_FUNCTION(getContactPointEvent)
        LUA_BIND_REFERENCE_TYPE_END
    };
}
