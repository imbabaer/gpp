#pragma once

#include "gep/math3d/vec3.h"

namespace gep
{
    class IRigidBody;

    struct CollisionArgs
    {
        struct CallbackSource
        {
            enum Enum
            {
                A,
                B,
                World
            };
        };

        CallbackSource::Enum source;
        IRigidBody* first;
        IRigidBody* second;

        CollisionArgs(CallbackSource::Enum source, IRigidBody* first, IRigidBody* second) :
            source(source),
            first(first),
            second(second)
        {
        }
    };

    struct ContactPointArgs : public CollisionArgs
    {
        //TODO: Implement me!

        ContactPointArgs(CallbackSource::Enum source, IRigidBody* first, IRigidBody* second) :
            CollisionArgs(source, first, second)
        {
        }
    };

    class IContactListener
    {
    public:
        virtual ~IContactListener(){}

        virtual void contactPointCallback( const ContactPointArgs& evt ) = 0;
        virtual void collisionAddedCallback( const CollisionArgs& evt ) = 0;
        virtual void collisionRemovedCallback( const CollisionArgs& evt ) = 0;
    };
}
