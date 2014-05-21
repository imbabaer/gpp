#pragma once

#include "gep/math3d/vec3.h"

namespace gep
{
    class ICollidable;

    struct RayCastInput
    {
        vec3 from;
        vec3 to;

        RayCastInput() :
            from(0.0f),
            to(0.0f)
        {
        }
    };

    struct RayCastOutput
    {
        float hitFraction;
        const ICollidable* hitEntity;

        bool hasHit() { return hitEntity != nullptr; }

        RayCastOutput() :
            hitFraction(1.0f),
            hitEntity(nullptr)
        {
        }
    };
}
