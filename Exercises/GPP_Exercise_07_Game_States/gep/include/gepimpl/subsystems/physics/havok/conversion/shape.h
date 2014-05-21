#pragma once

#include "gep/interfaces/physics/shape.h"

#include "gepimpl/subsystems/physics/havok/shape.h"

namespace gep {
namespace conversion {
namespace hk {

    inline hkpShape* to(IShape* in_gepShape)
    {
        hkpShape* result = nullptr;

        GEP_ASSERT(in_gepShape, "Shape is a nullptr!");

        switch (in_gepShape->getShapeType())
        {
        case ShapeType::Box:
            {
                auto* box = static_cast<BoxShape*>(in_gepShape);
                GEP_ASSERT(dynamic_cast<BoxShape*>(in_gepShape) != nullptr, "Shape type does not match the actual class type!");
                result = new hkpBoxShape(conversion::hk::to(box->getHalfExtents()));
            }
            break;
        case ShapeType::Sphere:
            {
                auto* sphere = static_cast<SphereShape*>(in_gepShape);
                GEP_ASSERT(dynamic_cast<SphereShape*>(in_gepShape) != nullptr, "Shape type does not match the actual class type!");
                result = new hkpSphereShape(sphere->getRadius());
            }
            break;
        case ShapeType::Triangle:
            {
                auto* mesh = static_cast<HavokMeshShape*>(in_gepShape);
                GEP_ASSERT(dynamic_cast<HavokMeshShape*>(in_gepShape) != nullptr, "Shape type does not match the actual class type!");
                result = mesh->getHkShape();
            }
            break;
        default:
            GEP_ASSERT(false, "Unsupported shape type!");
            break;
        }

        return result;
    }

    inline const hkpShape* to(const IShape* in_gepShape)
    {
        return to( const_cast<IShape*>(in_gepShape) );
    }

    //////////////////////////////////////////////////////////////////////////

    inline IShape* from(hkpShape* in_hkShape)
    {
        IShape* result = nullptr;
        switch (in_hkShape->getType())
        {
        case hkcdShapeType::BOX:
            {
                const auto* boxShape = static_cast<const hkpBoxShape*>(in_hkShape);
                result = GEP_NEW(g_stdAllocator, BoxShape)(conversion::hk::from(boxShape->getHalfExtents()));
            }
            break;
        case hkcdShapeType::SPHERE:
            {
                const auto* sphereShape = static_cast<const hkpSphereShape*>(in_hkShape);
                result = GEP_NEW(g_stdAllocator, SphereShape)(sphereShape->getRadius());
            }
            break;
        //TODO: Check if it is actually ok to put all of these into a HavokMeshShape!
        case hkcdShapeType::TRIANGLE:
        case hkcdShapeType::BV_COMPRESSED_MESH:
        case hkcdShapeType::CONVEX_VERTICES:
            {
                result = GEP_NEW(g_stdAllocator, HavokMeshShape)(in_hkShape);
            }
            break;
        default:
            GEP_ASSERT(false, "Unsupported shape type!");
            break;
        }

        return result;
    }

    inline const IShape* from(const hkpShape* in_hkShape)
    {
        return from( const_cast<hkpShape*>(in_hkShape) );
    }

}}} // namespace gep::conversion::hk
