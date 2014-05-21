#pragma once
#include "gep/interfaces/resourceManager.h"
#include "gep/interfaces/physics/shape.h"

namespace gep
{
    struct WorldCInfo;
    class IWorld;

    struct RigidBodyCInfo;
    class IRigidBody;

    struct CharacterRigidBodyCInfo;
    class ICharacterRigidBody;

    class ICollisionMesh;
    class IShape;

    //////////////////////////////////////////////////////////////////////////

    class IPhysicsFactory
    {
    public:
        virtual ~IPhysicsFactory(){}
        virtual void initialize() = 0;
        virtual void destroy() = 0;
        
        virtual IWorld* createWorld(const WorldCInfo& cinfo) const = 0;
        virtual IRigidBody* createRigidBody(const RigidBodyCInfo& cinfo) const = 0;
        virtual ICharacterRigidBody* createCharacterRigidBody(const CharacterRigidBodyCInfo& cinfo) const = 0;

        virtual ResourcePtr<ICollisionMesh> loadCollisionMesh(const char* path) = 0;

        BoxShape* createBox(vec3 halfExtends){ return new BoxShape(halfExtends); }
        SphereShape* createSphere(float radius){ return new SphereShape(radius); }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(createBox)
            LUA_BIND_FUNCTION(createSphere)
        LUA_BIND_REFERENCE_TYPE_END
    };


    class ICollisionMesh : public IResource
    {

    public:
        virtual ~ICollisionMesh(){}

        virtual IShape* getShape() = 0;
        virtual const IShape* getShape() const = 0;
    };
}
