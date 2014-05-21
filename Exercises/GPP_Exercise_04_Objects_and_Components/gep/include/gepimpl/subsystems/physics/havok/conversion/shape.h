#pragma once

#include "gep/interfaces/physics/shape.h"

#include "gepimpl/subsystems/physics/havok/shape.h"

namespace gep {
namespace conversion {
namespace hk {

    inline hkpShape* toHkShape(IShape* in_gepShape)
    {
        hkpShape* result = nullptr;

        switch (in_gepShape->getShapeType())
        {
        case ShapeType::Box:
            {
                auto* box = static_cast<BoxShape*>(in_gepShape);
                GEP_ASSERT(dynamic_cast<BoxShape*>(in_gepShape) != nullptr, "Shape type does not match the actual class type!");
                result = new hkpBoxShape(conversion::hk::toHkVector4(box->getHalfExtents()));
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

    inline const hkpShape* toHkShape(const IShape* in_gepShape)
    {
        return toHkShape( const_cast<IShape*>(in_gepShape) );
    }

    //////////////////////////////////////////////////////////////////////////

    inline IShape* fromHkShape(hkpShape* in_hkShape)
    {
        IShape* result = nullptr;
        switch (in_hkShape->getType())
        {
        case hkcdShapeType::BOX:
            {
                const auto* boxShape = static_cast<const hkpBoxShape*>(in_hkShape);
                result = new BoxShape(conversion::hk::fromHkVector4(boxShape->getHalfExtents()));
            }
            break;
        case hkcdShapeType::SPHERE:
            {
                const auto* sphereShape = static_cast<const hkpSphereShape*>(in_hkShape);
                result = new SphereShape(sphereShape->getRadius());
            }
            break;
        //TODO: Check if it is actually ok to put all of these into a HavokMeshShape!
        case hkcdShapeType::TRIANGLE:
        case hkcdShapeType::BV_COMPRESSED_MESH:
        case hkcdShapeType::CONVEX_VERTICES:
            {
                result = new HavokMeshShape(in_hkShape);
            }
            break;
        default:
            GEP_ASSERT(false, "Unsupported shape type!");
            break;
        }

        return result;
    }

    inline const IShape* fromHkShape(const hkpShape* in_hkShape)
    {
        return fromHkShape( const_cast<hkpShape*>(in_hkShape) );
    }

}}} // namespace gep::conversion::hk
