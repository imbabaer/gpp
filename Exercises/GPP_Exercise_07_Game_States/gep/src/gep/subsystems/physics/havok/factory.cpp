#include "stdafx.h"
#include "gepimpl/subsystems/physics/havok/factory.h"
#include "gep/memory/allocator.h"


gep::CollisionMesh::CollisionMesh() :
    m_pShape(nullptr)
{
}

gep::CollisionMesh::~CollisionMesh()
{
}

void gep::CollisionMesh::setShape(IShape* shape)
{
    m_pShape = shape;
}

gep::IShape* gep::CollisionMesh::getShape()
{
    return m_pShape;
}

const gep::IShape* gep::CollisionMesh::getShape() const
{
    return m_pShape;
}

const char* gep::CollisionMesh::getResourceType()
{
    throw std::logic_error("The method or operation is not implemented.");
}

gep::IResource* gep::CollisionMesh::getSuperResource()
{
    throw std::logic_error("The method or operation is not implemented.");
}

bool gep::CollisionMesh::isLoaded()
{
    throw std::logic_error("The method or operation is not implemented.");
}

gep::uint32 gep::CollisionMesh::getFinalizeOptions()
{
    return ResourceFinalize::NotRequired;
}

void gep::CollisionMesh::finalize()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void gep::CollisionMesh::unload()
{
    DELETE_AND_NULL(m_pShape);
}

void gep::CollisionMesh::setLoader(IResourceLoader* loader)
{
    GEP_ASSERT(loader != nullptr);
#ifdef _DEBUG
    m_pCollisionMeshFileLoader = dynamic_cast<CollisionMeshFileLoader*>(loader);
    GEP_ASSERT(m_pCollisionMeshFileLoader != nullptr);
#else
    m_pCollisionMeshFileLoader = static_cast<CollisionMeshFileLoader*>(loader);
#endif
}

gep::IResourceLoader* gep::CollisionMesh::getLoader()
{
    return m_pCollisionMeshFileLoader;
}

gep::CollisionMeshFileLoader::CollisionMeshFileLoader(const char* path) : m_path(path)
{

}

gep::CollisionMeshFileLoader::~CollisionMeshFileLoader()
{

}

void gep::CollisionMeshFileLoader::release()
{
    delete this;
}

gep::IResourceLoader* gep::CollisionMeshFileLoader::moveToHeap()
{
    return new CollisionMeshFileLoader(*this);
}

const char* gep::CollisionMeshFileLoader::getResourceId()
{
    return m_path.c_str();
}

const char* gep::CollisionMeshFileLoader::getResourceType()
{
    return "CollisionMesh";
}

void gep::CollisionMeshFileLoader::deleteResource(IResource* pResource)
{
    auto* actualResource = dynamic_cast<ICollisionMesh*>(pResource);
    GEP_ASSERT(actualResource != nullptr, "wrong type of resource to delete with this loader!");
    delete actualResource;
}

void gep::CollisionMeshFileLoader::postLoad(ResourcePtr<IResource> pResource)
{

}

gep::CollisionMesh* gep::CollisionMeshFileLoader::loadResource(CollisionMesh* pInPlace)
{
    CollisionMesh* result = nullptr;
    bool isInPlace = true;
    if (pInPlace == nullptr)
    {
        result = new CollisionMesh();
        isInPlace = false;
    }
    auto* havokLoader = g_resourceManager.getHavokResourceLoader();
    auto* container = havokLoader->load(m_path.c_str());
    GEP_ASSERT(container != nullptr, "Could not load asset! %s", m_path.c_str());

    if (container)
    {
        auto* physicsData = reinterpret_cast<hkpPhysicsData*>(container->findObjectByType(hkpPhysicsDataClass.getName()));
        GEP_ASSERT(physicsData != nullptr, "Unable to load physics data!");

        if (physicsData)
        {
            const auto& physicsSystems = physicsData->getPhysicsSystems();
            GEP_ASSERT(physicsSystems.getSize() == 1, "Wrong number of physics systems!");
            auto* body = physicsSystems[0]->getRigidBodies()[0];
            const auto* hkShape = body->getCollidable()->getShape();
            IShape* shape = conversion::hk::from(const_cast<hkpShape*>(hkShape));

            result->setShape(shape);
        }

    }
    return result;
}

gep::IResource* gep::CollisionMeshFileLoader::loadResource(IResource* pInPlace)
{
    return loadResource(dynamic_cast<CollisionMesh*>(pInPlace));
}

gep::HavokPhysicsFactory::HavokPhysicsFactory(IAllocator* allocator) :
    m_pAllocator(allocator)
{
    GEP_ASSERT(m_pAllocator, "Allocator cannot be nullptr!");
}

gep::HavokPhysicsFactory::~HavokPhysicsFactory()
{
}

//////////////////////////////////////////////////////////////////////////

void gep::HavokPhysicsFactory::initialize()
{
    g_globalManager.getResourceManager()->registerResourceType("CollisionMesh", nullptr);
}

void gep::HavokPhysicsFactory::destroy()
{

}

gep::IAllocator* gep::HavokPhysicsFactory::getAllocator()
{
    return m_pAllocator;
}

void gep::HavokPhysicsFactory::setAllocator(IAllocator* allocator)
{
    m_pAllocator = allocator;
}

gep::IWorld* gep::HavokPhysicsFactory::createWorld(const WorldCInfo& cinfo) const 
{
    GEP_ASSERT(m_pAllocator, "Allocator cannot be nullptr!");
    return GEP_NEW(m_pAllocator, HavokWorld)(cinfo);
}

gep::IRigidBody* gep::HavokPhysicsFactory::createRigidBody(const RigidBodyCInfo& cinfo) const 
{
    GEP_ASSERT(m_pAllocator, "Allocator cannot be nullptr!");
    HavokRigidBody* rb = GEP_NEW(m_pAllocator, HavokRigidBody)(cinfo);
    rb->initialize();
    return rb;
}

gep::ICharacterRigidBody* gep::HavokPhysicsFactory::createCharacterRigidBody(const CharacterRigidBodyCInfo& cinfo) const 
{
    GEP_ASSERT(m_pAllocator, "Allocator cannot be nullptr!");
    return GEP_NEW(m_pAllocator, HavokCharacterRigidBody)(cinfo);
}

gep::ResourcePtr<gep::ICollisionMesh> gep::HavokPhysicsFactory::loadCollisionMesh(const char* path)
{
    return g_globalManager.getResourceManager()->loadResource<CollisionMesh>(CollisionMeshFileLoader(path), LoadAsync::No);
}

gep::IShape* gep::HavokPhysicsFactory::loadCollisionMeshFromLua(const char* path)
{
	return loadCollisionMesh(path).get()->getShape();
}
