#pragma once

#include "gep/math3d/vec3.h"

namespace gep {
namespace conversion {
namespace hk {

    inline void toHkVector4(const vec3& vec, hkVector4& hkVec)
    {
        hkVec.set(
            vec.x,
            vec.y,
            vec.z);
    }

    inline hkVector4 toHkVector4(const vec3& vec)
    {
        return hkVector4(
            vec.x,
            vec.y,
            vec.z);
    }

    inline void fromHkVector4(const hkVector4& hkVec, vec3& vec)
    {
        vec.x = hkVec.getComponent<0>();
        vec.y = hkVec.getComponent<1>();
        vec.z = hkVec.getComponent<2>();
    }

    inline vec3 fromHkVector4(const hkVector4& vec)
    {
        return vec3(
            vec.getComponent<0>(),
            vec.getComponent<1>(),
            vec.getComponent<2>());
    }

}}} // namespace gep::conversion::hk
