#pragma once

#include "gepimpl/subsystems/physics/havok/conversion/vector.h"
#include "gepimpl/subsystems/physics/havok/conversion/quaternion.h"

namespace gep {
namespace conversion {
namespace hk {


    inline void fromHkTransform(const hkTransform& hkTrans, vec3& translation, Quaternion& rotation)
    {
        hkTransform trans = hkTrans; //HACK for fixing error when fromVector4 is called directly on hkTrans. TODO: Fix!
        fromHkVector4(trans.getTranslation(),translation);
        fromHkQuaternion(hkQuaternion(hkTrans.getRotation()), rotation);
    }

}}} // namespace gep::conversion::hk
