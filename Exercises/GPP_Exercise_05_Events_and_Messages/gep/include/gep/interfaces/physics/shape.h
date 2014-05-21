#pragma once
#include "gep/math3d/vec3.h"
#include "gep/ReferenceCounting.h"

namespace gep
{
    /// \brief Purely static struct that serves as an enum.
    struct ShapeType
    {
        enum Enum
        {
            Sphere = 0,
            Cylinder = 1,
            Triangle = 2,
            Box = 3,
            Capsule = 4,
            ConvexVertices = 5,
        };

        GEP_DISALLOW_CONSTRUCTION(ShapeType);
    };

    class IShape : public ReferenceCounted
    {
    public:
        virtual ~IShape(){}

        virtual ShapeType::Enum getShapeType() const = 0;
        LUA_BIND_REFERENCE_TYPE_BEGIN
        LUA_BIND_REFERENCE_TYPE_END
    };

    class BoxShape : public IShape
    {
        vec3 m_halfExtents;
    public:
        inline BoxShape(const vec3& halfExtents) : m_halfExtents(halfExtents) {}
        //inline BoxShape() : m_halfExtents(0.5f,0.5f,0.5f) {}

        inline ~BoxShape(){}

        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::Box; }

        inline vec3 getHalfExtents() const { return m_halfExtents; }
        inline void setHalfExtents(const vec3 & value) { m_halfExtents = value; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getHalfExtents)
            LUA_BIND_FUNCTION(setHalfExtents)
        LUA_BIND_REFERENCE_TYPE_END
    };

    class SphereShape : public IShape
    {
        float m_radius;
    public:
        SphereShape(float radius) : m_radius(radius) {}

        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::Sphere; }

        float getRadius() const { return m_radius; }
        void setRadius(float value) { m_radius = value; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getRadius)
            LUA_BIND_FUNCTION(setRadius)
        LUA_BIND_REFERENCE_TYPE_END
    };
}
