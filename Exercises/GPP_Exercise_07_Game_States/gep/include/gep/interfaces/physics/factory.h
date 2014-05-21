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

        virtual IAllocator* getAllocator() = 0;
        virtual void setAllocator(IAllocator* allocator) = 0;

        /// \brief Keep in mind that IWorld is a ReferenceCounted object. Use SmartPtrs whenever possible.
        virtual IWorld* createWorld(const WorldCInfo& cinfo) const = 0;

        /// \brief Keep in mind that IRigidBody is a ReferenceCounted object. Use SmartPtrs whenever possible.
        virtual IRigidBody* createRigidBody(const RigidBodyCInfo& cinfo) const = 0;

        /// \brief Keep in mind that ICharacterRigidBody is a ReferenceCounted object. Use SmartPtrs whenever possible.
        virtual ICharacterRigidBody* createCharacterRigidBody(const CharacterRigidBodyCInfo& cinfo) const = 0;

		virtual ResourcePtr<ICollisionMesh> loadCollisionMesh(const char* path) = 0;
		virtual IShape* loadCollisionMeshFromLua(const char* path) = 0;

        BoxShape* createBox(vec3 halfExtends){ return GEP_NEW(g_stdAllocator, BoxShape)(halfExtends); }
        SphereShape* createSphere(float radius){ return GEP_NEW(g_stdAllocator, SphereShape)(radius); }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(createWorld)
            LUA_BIND_FUNCTION(createRigidBody)
			LUA_BIND_FUNCTION(createBox)
			LUA_BIND_FUNCTION(createSphere)
			LUA_BIND_FUNCTION_NAMED(loadCollisionMeshFromLua, "loadCollisionMesh")
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
