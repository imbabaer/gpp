#pragma once

#include "gepimpl/subsystems/physics/havok/conversion/vector.h"
#include "gepimpl/subsystems/physics/havok/conversion/surfaceInfo.h"

namespace gep {
namespace conversion {
namespace hk {

    inline void toHkCharacterInput(const CharacterInput& in_from, hkpCharacterInput& out_to)
    {
        out_to.m_inputLR = in_from.inputLR;
        out_to.m_inputUD = in_from.inputUD;
        out_to.m_wantJump = in_from.wantJump;
        toHkVector4(in_from.up, out_to.m_up);
        toHkVector4(in_from.forward, out_to.m_forward);
        out_to.m_atLadder = in_from.atLadder;

        // step info
        {
            out_to.m_stepInfo.m_deltaTime = in_from.deltaTime;
            out_to.m_stepInfo.m_invDeltaTime = 1.0f/in_from.deltaTime;
        }

        toHkSurfaceInfo(in_from.surfaceInfo, out_to.m_surfaceInfo);

        toHkVector4(in_from.position, out_to.m_position);
        toHkVector4(in_from.velocity, out_to.m_velocity);
        toHkVector4(in_from.characterGravity, out_to.m_characterGravity);
    }

    inline void fromHkCharacterOutput(const hkpCharacterOutput& input, CharacterOutput& output)
    {
        fromHkVector4(input.m_velocity, output.velocity);
        fromHkVector4(input.m_inputVelocity, output.inputVelocity);
    }

}}} // namespace gep::conversion::hk
